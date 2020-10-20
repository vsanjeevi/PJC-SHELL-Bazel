/*
 * Extension to client_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Oct. 16. 2020
 * 
 * This extends the Client code to work with tuples that has 2 business data columns
 */

#include "client_tuple_impl.h"

#include <algorithm>
#include <iterator>

#include "absl/memory/memory.h"

namespace private_join_and_compute {
//YAR::Edit: Constructor using tuple instead of pair
PrivateIntersectionSumProtocolClientTupleImpl::PrivateIntersectionSumProtocolClientTupleImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<BigNum>, std::vector<BigNum>>& table, int32_t modulus_size)
    :PrivateIntersectionSumProtocolClientImpl(
      ctx, std::get<0>(table),std::get<1>(table),modulus_size),table_(table)
       {}

//YAR::Add : Refactoring
// Moving the data format specific operations to a helper function
// This will allow for specialization through inheritance  
StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> 
PrivateIntersectionSumProtocolClientTupleImpl::EncryptCol(){
  auto ids = std::get<0>(table_);
  auto col_1 = std::get<1>(table_);
  auto col_2 = std::get<2>(table_);

  PrivateIntersectionSumClientMessage::ClientRoundOne result;

  for (size_t i = 0; i < ids.size(); i++) {
    EncryptedElement* element = result.mutable_encrypted_set()->add_elements();
    StatusOr<std::string> encrypted = ec_cipher_->Encrypt(ids[i]);
    if (!encrypted.ok()) {
      return encrypted.status();
    }
    *element->mutable_element() = encrypted.value();
    //YAR::Note : This is where the business keys are encrypted using homomorphic encryption
    //col_1
    StatusOr<BigNum> value_1 = private_paillier_->Encrypt(col_1[i]);
    if (!value_1.ok()) {
      return value_1.status();
    }
    *element->mutable_associated_data_1() = value_1.value().ToBytes();

    //col_2
    StatusOr<BigNum> value_2 = private_paillier_->Encrypt(col_2[i]);
    if (!value_2.ok()) {
      return value_2.status();
    }
    *element->mutable_associated_data_2() = value_2.value().ToBytes();

  }
  return result;
}

//YAR::Add : Refactoring
//This method can directly set the internal variables
Status PrivateIntersectionSumProtocolClientTupleImpl::DecryptSum(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptSum before ReEncryptSet.");
  }

  StatusOr<BigNum> sum_1 = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum_1()));
  if (!sum_1.ok()) {
    return sum_1.status();
  } 
  intersection_sum_1_ = sum_1.value(); //BigNum --> convert to int to print

  StatusOr<BigNum> sum_2 = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum_2()));
  if (!sum_2.ok()) {
    return sum_2.status();
  }
  intersection_sum_2_ = sum_2.value();

  return OkStatus();
}



Status PrivateIntersectionSumProtocolClientTupleImpl::PrintOutput() {
  if (!protocol_finished()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolClientImpl: Not ready to print the "
        "output yet.");
  }
  //YAR::Edit : Got two sums
  auto maybe_converted_intersection_sum_1 = intersection_sum_1_.ToIntValue();
  if (!maybe_converted_intersection_sum_1.ok()) {
    return maybe_converted_intersection_sum_1.status();
  }
  auto maybe_converted_intersection_sum_2 = intersection_sum_2_.ToIntValue();
  if (!maybe_converted_intersection_sum_2.ok()) {
    return maybe_converted_intersection_sum_2.status();
  }

  //YAR::Edit : 2 sums to be displayed
  std::cout << "Client: The intersection size is " << intersection_size_
            << " and the intersection-sum-1 is "
            << maybe_converted_intersection_sum_1.value() 
            << " and the intersection-sum-2 is "
            << maybe_converted_intersection_sum_2.value() 
            << std::endl;
  return OkStatus();
}    