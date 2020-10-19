/*
 * Extension to client_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Oct. 16. 2020
 * 
 * This extends the Client code to work with tuples that has 2 business data columns
 */

#include "client_impl_tuple.h"

#include <algorithm>
#include <iterator>

#include "absl/memory/memory.h"

namespace private_join_and_compute {
//YAR::Edit: Constructor using tuple instead of pair
PrivateIntersectionSumProtocolClientTupleImpl::
    PrivateIntersectionSumProtocolClientImpl:PrivateIntersectionSumProtocolClientTupleImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<BigNum>, std::vector<BigNum>>& table, int32_t modulus_size)
    :PrivateIntersectionSumProtocolClientImpl(
      ctx, std::get<0>(table),std::get<1>(table),modulus_size),table_(table)
       {}
    
    