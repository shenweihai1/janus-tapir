#pragma once

#include "__dep__.h"
#include "config.h"
#include "communicator.h"

namespace rococo {

class ClientControlServiceImpl;
class TxnGenerator;
class CoordinatorBase;
class Frame;
class Coordinator;
class TxnRegistry;
class TxnReply;

class ClientWorker {
 public:
  Frame* frame_;
  Communicator* commo_;
  cliid_t cli_id_;
  int32_t benchmark;
  int32_t mode;
  bool batch_start;
  uint32_t id;
  uint32_t duration;
  ClientControlServiceImpl *ccsi;
  uint32_t n_concurrent_;
  rrr::Mutex finish_mutex;
  rrr::CondVar finish_cond;

  // coordinators_{mutex, cond} synchronization currently only used for open clients
  rrr::Mutex coordinators_mutex;
  rrr::CondVar coordinators_cond;
  vector<CoordinatorBase*> coos_ = {};
  
  std::vector<uint64_t> cli2cli_;

  std::atomic<uint32_t> num_txn, success, num_try;
  TxnGenerator * txn_generator_;
  Timer *timer_;
  TxnRegistry* txn_reg_ = nullptr;
  Config* config_;
  Config::SiteInfo& my_site_;
  vector<string> servers_;
 public:
  ClientWorker(uint32_t id,
               Config::SiteInfo &site_info,
               Config *config,
               ClientControlServiceImpl *ccsi);
  ClientWorker() = delete;
  ~ClientWorker();
  void work();
 protected:
  Coordinator* CreateCoordinator(uint16_t offset_id);
  void DispatchRequest(Coordinator *coo);
  void RequestDone(Coordinator* coo, uint64_t, TxnReply &txn_reply);

  Coordinator* GetFreeCoordinator();
};
} // namespace rococo


