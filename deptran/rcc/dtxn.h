#pragma once
#include "../__dep__.h"
#include "../dtxn.h"
#include "../txn_chopper.h"

#define PHASE_RCC_DISPATCH (1)
#define PHASE_RCC_COMMIT (2)

namespace rococo {
class RccDTxn: public DTxn {
 public:
  int8_t status_ = TXN_UKN;
  vector<SimpleCommand> dreqs_ = {};
  Vertex<RccDTxn>* tv_{nullptr};
  RccGraph* graph_{nullptr};
  TxnOutput *ptr_output_repy_ = nullptr;
  TxnOutput output_ = {};
  vector<TxnInfo *> conflict_txns_ = {}; // This is read-only transaction
  function<void(int)> finish_reply_callback_ =  [] (int) -> void {verify(0);};
  bool commit_request_received_ = false;
  bool read_only_ = false;
  bool committed = false;
  bool aborted = false;
  bool __debug_replied = false;

  RccDTxn(txnid_t tid, Scheduler *mgr, bool ro);

  RccDTxn(txnid_t id);

  virtual ~RccDTxn() {}

  virtual void DispatchExecute(const SimpleCommand &cmd,
                               int *res,
                               map<int32_t, Value> *output);

  virtual void CommitExecute();

  virtual void Abort();

  virtual void ReplyFinishOk();

  bool ReadColumn(mdb::Row *row,
                  mdb::column_id_t col_id,
                  Value *value,
                  int hint_flag) override;

  bool WriteColumn(Row *row,
                   column_id_t col_id,
                   const Value &value,
                   int hint_flag = TXN_INSTANT) override;

  void TraceDep(Row* row, column_id_t col_id, int hint_flag);

    virtual bool start_exe_itfr(
      defer_t defer,
      TxnHandler &handler,
      const SimpleCommand& cmd,
      map<int32_t, Value> *output
  );

  // TODO remove
//  virtual void start(
//      const RequestHeader &header,
//      const std::vector <mdb::Value> &input,
//      bool *deferred,
//      ChopStartResponse *res
//  );

  virtual void start_ro(const SimpleCommand&,
                        map<int32_t, Value> &output,
                        rrr::DeferredReply *defer);

//  virtual void commit_anc_finish(
//      Vertex <TxnInfo> *v,
//      rrr::DeferredReply *defer
//  );
//
//  virtual void commit_scc_anc_commit(
//      Vertex <TxnInfo> *v,
//      rrr::DeferredReply *defer
//  );
//
//  void exe_deferred(vector <std::pair<RequestHeader,
//                                      map<int32_t, Value> > > &outputs);


  virtual mdb::Row *CreateRow(
      const mdb::Schema *schema,
      const std::vector<mdb::Value> &values) {
    return RCCRow::create(schema, values);
  }

  virtual void kiss(mdb::Row *r,
                    int col,
                    bool immediate);

  virtual bool UpdateStatus(int s) {
    if (status_ < s) {
      status_ = s;
      return true;
    } else {
      return false;
    }
  }

  // below is copied from txn-info
// private:
//  int8_t status_{TXN_UKN};

 public:
  txnid_t txn_id_;
  std::set<uint32_t> partition_;
  std::vector<uint64_t> pieces_;
  bool fully_dispatched{false};
  bool executed_{false};
  bool committed_{false};
  bool during_commit = false;
  bool during_asking = false;
  bool inquire_acked_ = false;
  bool all_anc_cmt_hint{false};
//  RccGraph* graph_{nullptr};

  vector<RccGraph*> graphs_for_inquire_{};
  vector<function<void()>> callbacks_for_inquire_{};

  ChopFinishResponse *res = nullptr;
//
//  RccDTxn(uint64_t id) : DTxn(id, nullptr) {
//    txn_id_ = id;
//  }
//
//  ~RccDTxn() {
//    Log_debug("txn info destruct, id: %llx", txn_id_);
//    txn_id_ = 0; // for debug purpose
//  }

  inline int8_t get_status() const {
    return status_;
  }

  inline int8_t status() const {
    return status_;
  }

  inline bool IsAborted() const {
    return (status_ & TXN_ABT);
  }

  inline bool IsCommitting() const {
    return (status_ & TXN_CMT);
  }

  inline bool IsDecided() const {
    return (status_ & TXN_DCD);
  }

  inline bool IsExecuted() const {
    if (executed_) verify(IsDecided());
    return executed_;
  }

  bool Involve(svrid_t id) {
    auto it = partition_.find(id);
    if (it == partition_.end()) {
      return false;
    } else {
      return true;
    }
  }

  uint32_t random_server() {
    verify(partition_.size() > 0);
    uint32_t i = RandomGenerator::rand(0, partition_.size() - 1);
    auto it = partition_.begin();
    std::advance(it, i);
    uint32_t id = *(partition_.begin());
    Log::debug("random a related server, id: %x", id);
    return id;
  }

  bool operator<(const RccDTxn &rhs) const {
    return txn_id_ < rhs.txn_id_;
  }

  inline uint64_t id() {
    return txn_id_;
  }

  inline void set_id(uint64_t id) {
    txn_id_ = id;
  }

  inline void union_data(const RccDTxn &ti,
                         bool trigger = false,
                         bool server = false) {
    partition_.insert(ti.partition_.begin(), ti.partition_.end());
    union_status(ti.status_, trigger, server);
  }

  void trigger() {
    verify(0);
    for (auto &kv: events_) {
      if (kv.first <= status_) {
        while (kv.second.size() > 0) {
          DragonBall *ball = kv.second.back();
          kv.second.pop_back();
          ball->trigger();
        }
      }
    }
  }

  void union_status(int8_t status,
                    bool is_trigger = false,
                    bool is_server = false) {

    if (false) {
      // TODO
      // I cannot change the status of this txn.
    } else {
#ifdef DEBUG_CODE
      verify((status_ | status) >= status_);
#endif
      status_ |= status;

    }
    if (is_trigger) {
      trigger();
    }
  }

  void register_event(int8_t status, DragonBall *ball) {
    if (status_ >= status) {
      ball->trigger();
    }
    else {
      events_[status].push_back(ball);
    }
  }

  // a simple simple state machine, don't have to marshal
  int32_t wait_finish_ = 0;
  int32_t wait_commit_ = 0;

  std::map<int8_t, std::vector<DragonBall *> > events_;


};


inline rrr::Marshal &operator<<(rrr::Marshal &m, const RccDTxn &ti) {
  m << ti.txn_id_ << ti.status() << ti.partition_;
  return m;
}

inline rrr::Marshal &operator>>(rrr::Marshal &m, RccDTxn &ti) {
  int8_t status;
  m >> ti.txn_id_ >> status >> ti.partition_;
  ti.union_status(status);
  return m;
}

} // namespace rococo
