#include "deptran/__dep__.h"
#include "../tpcc/piece.h"
#include "piece.h"

namespace rococo {
  char RW_BENCHMARK_TABLE[] = "history";

  void RWPiece::reg_all() {
      reg_pieces();
  }

  void RWPiece::reg_pieces() {

    SHARD_PIE(RW_BENCHMARK_R_TXN, RW_BENCHMARK_R_TXN_0,
              TPCC_TB_HISTORY, TPCC_VAR_H_KEY)
    BEGIN_PIE(RW_BENCHMARK_R_TXN, RW_BENCHMARK_R_TXN_0, DF_NO) {
      mdb::MultiBlob buf(1);
      auto tbl = dtxn->GetTable(RW_BENCHMARK_TABLE);

      // 4 reads
      mdb::Row *roww;
      const int num_partitions = Config::GetConfig()->replica_groups_.size();
      int32_t a = cmd.input.at(0).get_i32() % num_partitions ;
      for (int i=0; i<4; ++i) {
          int32_t b = cmd.input.at(i + 1).get_i32() % num_partitions ;
          int32_t c = cmd.input.at(i + 1).get_i32() - (b-a) ;
          Value vc(c) ;
          buf[0] = vc.get_blob();
          roww = dtxn->Query(tbl, buf);
          dtxn->ReadColumn(roww,1, &output[i], TXN_BYPASS);
      }
      *res = SUCCESS;
    
      /*
      mdb::MultiBlob buf(1);
      Value result(0);
      verify(cmd.input.size()==1);
      auto id = cmd.input[0].get_i64();
      buf[0] = cmd.input[0].get_blob();
      auto tbl = dtxn->GetTable(RW_BENCHMARK_TABLE);
      auto row = dtxn->Query(tbl, buf);
      dtxn->ReadColumn(row, 1, &result, TXN_BYPASS);
      output[0] = result;
      *res = SUCCESS;*/
      return;
    } END_PIE

    SHARD_PIE(RW_BENCHMARK_W_TXN, RW_BENCHMARK_W_TXN_0,
              TPCC_TB_HISTORY, TPCC_VAR_H_KEY)
    BEGIN_PIE(RW_BENCHMARK_W_TXN, RW_BENCHMARK_W_TXN_0, DF_REAL) {
      mdb::MultiBlob buf(1);
      auto tbl = dtxn->GetTable(RW_BENCHMARK_TABLE);

      mdb::Row *roww;
      const int num_partitions = Config::GetConfig()->replica_groups_.size();
      int32_t a = cmd.input.at(0).get_i32() % num_partitions ;
      // 4 RMW
      for (int i=0; i<4; ++i) {
          int32_t b = cmd.input.at(i + 1).get_i32() % num_partitions ;
          int32_t c = cmd.input.at(i + 1).get_i32() - (b-a) ;
          Value vc(c) ;
          //Log_info("the key: a,b,c:%d,%d,%d, partitions:%d", a,b,c+(b-a), num_partitions);
          buf[0] = vc.get_blob();
          roww = dtxn->Query(tbl, buf);
          dtxn->ReadColumn(roww,1, &output[i], TXN_BYPASS);

          output[i].set_i32(output[i].get_i32() + 1);
          dtxn->WriteColumn(roww, 1, output[i], TXN_DEFERRED);
      }

      *res = SUCCESS;
      /*mdb::MultiBlob buf(1);
      Value result(0);
      verify(cmd.input.size() == 1);
      auto id = cmd.input[0].get_i64();
      buf[0] = cmd.input[0].get_blob();
      auto tbl = dtxn->GetTable(RW_BENCHMARK_TABLE);
      auto row = dtxn->Query(tbl, buf);
      dtxn->ReadColumn(row, 1, &result, TXN_BYPASS);
      result.set_i32(result.get_i32()+1);
      dtxn->WriteColumn(row, 1, result, TXN_DEFERRED);
      *res = SUCCESS;*/
      return;
    } END_PIE
  }
}

