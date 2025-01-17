#include "../rcc/dtxn.h"
#include "../txn_chopper.h"
#include "../rcc/graph_marshaler.h"
#include "dep_graph.h"
#include "commo.h"
#include "marshallable.h"
#include "brq_srpc.h"

namespace rococo {

void BrqCommo::SendDispatch(vector<SimpleCommand> &cmd,
                            const function<void(int res,
                                                TxnOutput& cmd,
                                                RccGraph& graph)>& callback) {
  rrr::FutureAttr fuattr;
  auto tid = cmd[0].root_id_;
  auto par_id = cmd[0].partition_id_;
  std::function<void(Future*)> cb =
      [callback, tid, par_id] (Future *fu) {
        int res;
        TxnOutput output;
        Marshallable graph;
        fu->get_reply() >> res >> output >> graph;
        if (graph.rtti_ == Marshallable::EMPTY_GRAPH) {
          RccGraph rgraph;
          auto v = rgraph.CreateV(tid);
          RccDTxn& info = v->Get();
          info.partition_.insert(par_id);
          callback(res, output, rgraph);
        } else if (graph.rtti_ == Marshallable::RCC_GRAPH) {
          callback(res, output, dynamic_cast<RccGraph&>(*graph.ptr()));
        } else {

        }
      };
  fuattr.callback = cb;
  auto proxy = (BrqProxy*)NearestProxyForPartition(
      cmd[0].PartitionId()).second;
  Log_debug("dispatch to %ld", cmd[0].PartitionId());
//  verify(cmd.type_ > 0);
//  verify(cmd.root_type_ > 0);
  Future::safe_release(proxy->async_Dispatch(cmd, fuattr));
}

void BrqCommo::SendHandoutRo(SimpleCommand &cmd,
                             const function<void(int res,
                                                 SimpleCommand& cmd,
                                                 map<int, mdb::version_t>& vers)>&) {
  verify(0);
}

void BrqCommo::SendFinish(parid_t pid,
                          txnid_t tid,
                          RccGraph& graph,
                          const function<void(TxnOutput& output)> &callback) {
  verify(0);
  FutureAttr fuattr;
  function<void(Future*)> cb = [callback] (Future* fu) {
    int32_t res;
    TxnOutput outputs;
    fu->get_reply() >> res >> outputs;
    callback(outputs);
  };
  fuattr.callback = cb;
  auto proxy = (BrqProxy*)NearestProxyForPartition(pid).second;
  Future::safe_release(proxy->async_Commit(tid, (BrqGraph&)graph, fuattr));
}

void BrqCommo::SendInquire(parid_t pid,
                           txnid_t tid,
                           const function<void(RccGraph& graph)>& callback) {
  FutureAttr fuattr;
  function<void(Future*)> cb = [callback] (Future* fu) {
    Marshallable graph;
    fu->get_reply() >> graph;
    callback(dynamic_cast<RccGraph&>(*graph.ptr()));
  };
  fuattr.callback = cb;
  // TODO fix.
  auto proxy = (BrqProxy*)NearestProxyForPartition(pid).second;
  Future::safe_release(proxy->async_Inquire(tid, fuattr));
}

bool BrqCommo::IsGraphOrphan(RccGraph& graph, txnid_t cmd_id) {
  if (graph.size() == 1) {
    RccVertex* v = graph.FindV(cmd_id);
    verify(v);
    verify(v->incoming_.size() == 0);
    verify(v->outgoing_.size() == 0);
    return true;
  } else {
    return false;
  }
}

void BrqCommo::BroadcastPreAccept(parid_t par_id,
                                  txnid_t cmd_id,
                                  ballot_t ballot,
                                  vector<SimpleCommand>& cmds,
                                  RccGraph& graph,
                                  const function<void(int, RccGraph*)> &callback) {
  verify(rpc_par_proxies_.find(par_id) != rpc_par_proxies_.end());

  bool skip_graph = IsGraphOrphan(graph, cmd_id);

  for (auto &p : rpc_par_proxies_[par_id]) {
    auto proxy = (BrqProxy*)(p.second);
    verify(proxy != nullptr);
    FutureAttr fuattr;
    fuattr.callback = [callback] (Future* fu) {
      int32_t res;
      Marshallable* graph = new Marshallable;
      fu->get_reply() >> res >> *graph;
      callback(res, dynamic_cast<RccGraph*>(graph->ptr().get()));
    };
    verify(cmd_id > 0);
    if (skip_graph) {
      Future::safe_release(proxy->async_PreAcceptWoGraph(cmd_id,
                                                         cmds,
                                                         fuattr));
    } else {
      Future::safe_release(proxy->async_PreAccept(cmd_id,
                                                  cmds,
                                                  graph,
                                                  fuattr));
    }
  }
}

void BrqCommo::BroadcastAccept(parid_t par_id,
                               txnid_t cmd_id,
                               ballot_t ballot,
                               RccGraph& graph,
                               const function<void(int)> &callback) {
  verify(rpc_par_proxies_.find(par_id) != rpc_par_proxies_.end());
  for (auto &p : rpc_par_proxies_[par_id]) {
    auto proxy = (BrqProxy*)(p.second);
    verify(proxy != nullptr);
    FutureAttr fuattr;
    fuattr.callback = [callback] (Future* fu) {
      int32_t res;
      fu->get_reply() >> res;
      callback(res);
    };
    verify(cmd_id > 0);
    Future::safe_release(proxy->async_Accept(cmd_id,
                                             ballot,
                                             graph,
                                             fuattr));
  }
}


void BrqCommo::BroadcastCommit(parid_t par_id,
                               txnid_t cmd_id,
                               RccGraph& graph,
                               const function<void(int32_t, TxnOutput&)>
                               &callback) {
  bool skip_graph = IsGraphOrphan(graph, cmd_id);

  verify(rpc_par_proxies_.find(par_id) != rpc_par_proxies_.end());
  for (auto &p : rpc_par_proxies_[par_id]) {
    auto proxy = (BrqProxy*)(p.second);
    verify(proxy != nullptr);
    FutureAttr fuattr;
    fuattr.callback = [callback] (Future* fu) {
      int32_t res;
      TxnOutput output;
      fu->get_reply() >> res >> output;
      callback(res, output);
    };
    verify(cmd_id > 0);
    if (skip_graph) {
      Future::safe_release(proxy->async_CommitWoGraph(cmd_id, fuattr));
    } else {
      Future::safe_release(proxy->async_Commit(cmd_id, graph, fuattr));
    }
  }
}

} // namespace rococo