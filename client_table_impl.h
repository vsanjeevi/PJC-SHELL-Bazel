/*
 * Extension to client_impl for sharing N business data columns
 * Author:          Yuva Athur
 * Created Date:    Oct. 16. 2020
 * 
 * ***** Incomplete implementation **************
 * 
 * Follows the same implementation approach as PrivateIntersectionSumProtocolClientImpl
 *  Extending to a vector of data columns
 *  Each data column is expected to have an aggregation operation
 * 
 * First Design: Considered inheritance approach
 *  Challenges: Pair to extension to 
 *  
 */

#ifndef OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TABLE_IMPL_H_
#define OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TABLE_IMPL_H_

#include "crypto/context.h"
#include "crypto/paillier.h"
#include "match.pb.h"
#include "message_sink.h"
#include "private_intersection_sum.pb.h"
#include "private_join_and_compute.pb.h"
#include "protocol_client.h"
#include "util/status.inc"
#include "crypto/ec_commutative_cipher.h"

namespace private_join_and_compute {
class PrivateIntersectionSumProtocolClientTableImpl : public ProtocolClient {
 public:

  PrivateIntersectionSumProtocolClientTableImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<std::vector<BigNum>>>& table, int32_t modulus_size);

  // Generates the StartProtocol message and sends it on the message sink.
  Status StartProtocol(
      MessageSink<ClientMessage>* client_message_sink) override;

  // Replicates the behavior of PrivateIntersectionSumProtocolClientImpl
  Status Handle(const ServerMessage& server_message,
                MessageSink<ClientMessage>* client_message_sink) override;


  // Prints the result, namely the intersection size and the intersection sum.
  Status PrintOutput() override;

  // Utility functions for testing.
  int64_t intersection_size() const { return intersection_size_; }
  const BigNum& intersection_sum(int32_t idx) {return intersection_vec_[idx];}
  bool protocol_finished() override { return protocol_finished_; }


 protected:
  // Replicates the behavior of PrivateIntersectionSumProtocolClientImpl
  virtual StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> ReEncryptSet(
      const PrivateIntersectionSumServerMessage::ServerRoundOne&
          server_message);

  // Replicates the behavior of PrivateIntersectionSumProtocolClientImpl
  virtual StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> EncryptCol();

  // Replicates the behavior of PrivateIntersectionSumProtocolClientImpl
  virtual Status DecryptResult(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message); 

  Context* ctx_;  // not owned

  //table with many (dynamic) columns
  std::tuple<std::vector<std::string>, std::vector<std::vector<BigNum>>> table_;


  //extending to multiple sums
  std::vector<BigNum> intersection_aggregates_;

};
}  // namespace private_join_and_compute
#endif  // OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TABLE_IMPL_H_
