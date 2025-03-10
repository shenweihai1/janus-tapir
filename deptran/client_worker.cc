#include "client_worker.h"
#include "frame.h"
#include "txn_chopper.h"
#include "coordinator.h"
#include "piece.h"
#include "benchmark_control_rpc.h"
#include <chrono>

namespace rococo {

uint64_t GetNowInns() {
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    auto value = now_ns.time_since_epoch();
    return (uint64_t) value.count();
}

ClientWorker::~ClientWorker() {
  if (txn_generator_) {
    delete txn_generator_;
  }
  for (auto c : coos_) {
    delete c;
  }
  coos_.clear();
}

void ClientWorker::RequestDone(Coordinator* coo, uint64_t dispatch_time_, TxnReply &txn_reply) {
  verify(coo != nullptr);

  if (txn_reply.res_ == SUCCESS)
    success++;
  num_txn++;
  num_try.fetch_add(txn_reply.n_try_);

  finish_mutex.lock();
  cli2cli_.push_back(GetNowInns() - dispatch_time_);
  finish_mutex.unlock();

  bool have_more_time = timer_->elapsed() < duration;

  if (config_->client_type_ == Config::Open) {
    coordinators_mutex.lock();
    coos_.push_back(coo);
    coordinators_cond.signal();
    coordinators_mutex.unlock();
  }

  Log_debug("elapsed: %2.2f; duration: %d", timer_->elapsed(), duration);
  if (have_more_time && config_->client_type_ == Config::Open) {
    finish_mutex.lock();
    n_concurrent_--;
    Log_debug("open client -- num_outstanding: %d", n_concurrent_);
    finish_mutex.unlock();
  } else if (have_more_time && config_->client_type_ == Config::Closed) {
    Log_debug("there is still time to issue another request. continue.");
    DispatchRequest(coo);
  } else if (!have_more_time) {
    Log_debug("times up. stop.");
    Log_debug("n_concurrent_ = %d", n_concurrent_);
    finish_mutex.lock();
    n_concurrent_--;
    if (n_concurrent_ == 0) {
      Log_debug("all coordinators finished... signal done");
      finish_cond.signal();
    } else {
      Log_debug("waiting for %d more coordinators to finish", n_concurrent_);
    }
    finish_mutex.unlock();
  }
}

Coordinator* ClientWorker::CreateCoordinator(uint16_t offset_id) {
  cooid_t coo_id = cli_id_;
  coo_id = (coo_id << 16) + offset_id;
  auto coo = frame_->CreateCoord(coo_id,
                                 config_,
                                 benchmark,
                                 ccsi,
                                 id,
                                 txn_reg_);
  coo->loc_id_ = my_site_.locale_id;
  coo->commo_ = commo_;
  Log_debug("coordinator %d created.", coo_id);
  coos_.push_back(coo);
  return coo;
}

void ClientWorker::work() {
  Log_debug("%s", __FUNCTION__);
  verify(coos_.size() == 0);
  txn_reg_ = new TxnRegistry();
  Piece *piece = Piece::get_piece(benchmark);
  piece->txn_reg_ = txn_reg_;
  piece->reg_all();

  if (ccsi) ccsi->wait_for_start(id);
  Log_debug("after wait for start");

  timer_ = new Timer();
  timer_->start();

  if (config_->client_type_ == Config::Closed) {
    verify(n_concurrent_ > 0);
    for (uint32_t n_txn = 0; n_txn < n_concurrent_; n_txn++) {
      auto coo = CreateCoordinator(n_txn);
      DispatchRequest(coo);
    }
  } else {
    const double wait_time = 1.0/(double)config_->client_rate_;
    Log_debug("wait time %.10f", wait_time);
    uint64_t txn_count = 0;
    std::function<void()> do_dispatch = [&]() {
      double tps=0;
      do {
        txn_count++;
        tps = txn_count / timer_->elapsed();
        finish_mutex.lock();
        n_concurrent_++;
        finish_mutex.unlock();
        DispatchRequest(GetFreeCoordinator());
        Log_debug("client tps: %2.2f", tps);
      } while (tps < config_->client_rate_);
    };

    for (uint16_t i = 0; i < n_concurrent_; i++) {
      CreateCoordinator(i);
    }

    n_concurrent_ = 0;
    while (timer_->elapsed() < duration) {
      do_dispatch();
      std::this_thread::sleep_for(std::chrono::duration<double>(wait_time));
    }
    Log_debug("exit client dispatch loop...");
  }

  finish_mutex.lock();
  while (n_concurrent_ > 0) {
    Log_debug("wait for finish... %d", n_concurrent_);
    finish_cond.wait(finish_mutex);
  }
  finish_mutex.unlock();

  Log_info("Finish:\nTotal: %u, Commit: %u, Attempts: %u, Running for %u\n",
           num_txn.load(),
           success.load(),
           num_try.load(),
           Config::GetConfig()->get_duration());
  Log_info("Finish:\nCommit2:\t%u\tClient-id:%d\n",
           success.load(),
           cli_id_);
  fflush(stderr);
  fflush(stdout);

  //for (auto c : coos_) {
  //  for (int i=0; i<c->cli2cli_.size(); i++){
  //    cli2cli_.push_back(c->cli2cli_[i]);
  //  }
  //}

  std::sort(cli2cli_.begin(), cli2cli_.end());
  int index = 0;
  Log_info("Collected lat: %d", cli2cli_.size());
  index = static_cast<int>(0.99 * (cli2cli_.size() - 1) + 0.5);
  Log_info("The 99p lat: %fms", cli2cli_[index]/1000.0/1000.0);
  index = static_cast<int>(0.95 * (cli2cli_.size() - 1) + 0.5);
  Log_info("The 95p lat: %fms", cli2cli_[index]/1000.0/1000.0);
  index = static_cast<int>(0.50 * (cli2cli_.size() - 1) + 0.5);
  Log_info("The 50p lat: %fms", cli2cli_[index]/1000.0/1000.0);
  index = static_cast<int>(0.1 * (cli2cli_.size() - 1) + 0.5);
  Log_info("The 10p lat: %fms", cli2cli_[index]/1000.0/1000.0);


  if (ccsi) {
    Log_info("%s: wait_for_shutdown at client %d", __FUNCTION__, cli_id_);
    ccsi->wait_for_shutdown();
  }
  delete timer_;
  return;
}

  void ClientWorker::DispatchRequest(Coordinator *coo) {
    TxnRequest req;
    txn_generator_->GetTxnReq(&req, coo->coo_id_);
    uint64_t dispatch_time_ = GetNowInns();
    req.callback_ = std::bind(&rococo::ClientWorker::RequestDone,
                              this,
                              coo,
			      dispatch_time_,
                              std::placeholders::_1);
    coo->do_one(req);
  }

  ClientWorker::ClientWorker(uint32_t id,
                           Config::SiteInfo &site_info,
                           Config *config,
                           ClientControlServiceImpl *ccsi)
    : id(id),
      my_site_(site_info),
      config_(config),
      cli_id_(site_info.id),
      benchmark(config->get_benchmark()),
      mode(config->get_mode()),
      duration(config->get_duration()),
      ccsi(ccsi),
      n_concurrent_(config->get_concurrent_txn()) {
  frame_ = Frame::GetFrame(config->cc_mode_);
  txn_generator_ = frame_->CreateTxnGenerator();
  config->get_all_site_addr(servers_);
  num_txn.store(0);
  success.store(0);
  num_try.store(0);
  commo_ = frame_->CreateCommo();
  commo_->loc_id_ = my_site_.locale_id;
}

  Coordinator* ClientWorker::GetFreeCoordinator() {
    coordinators_mutex.lock();
    while (coos_.size() == 0) {
      coordinators_cond.wait(coordinators_mutex);
    }
    auto coo = coos_.back();
    coos_.pop_back();
    coordinators_mutex.unlock();
    return dynamic_cast<Coordinator*>(coo);
  }


} // namespace rococo


