/*
 * Extension to client_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Oct. 16. 2020
 * 
 * This extends the Client code to work with tuples that has 2 business data columns
 */

#ifndef OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_
#define OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_

#include "client_impl.h"

namespace private_join_and_compute {
class PrivateIntersectionSumProtocolClientTupleImpl : public PrivateIntersectionSumProtocolClientImpl {
 public:

  PrivateIntersectionSumProtocolClientTupleImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<BigNum>, std::vector<BigNum>>& table, int32_t modulus_size);

  const BigNum& intersection_sum_1() const { return intersection_sum_1_; }
  const BigNum& intersection_sum_2() const { return intersection_sum_2_; }
 
 protected:
   StatusOr<std::tuple<int64_t, BigNum, BigNum>>DecryptSum(
      const PrivateIntersectionSumServerMessage::ServerRoundTwo&
          server_message);

  //table with 2 columns
  std::tuple<std::vector<std::string>, std::vector<BigNum>, std::vector<BigNum>> table_;

  //extending to 2 sums
  BigNum intersection_sum_1_;
  BigNum intersection_sum_2_;


}  // namespace private_join_and_compute
#endif  // OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_
