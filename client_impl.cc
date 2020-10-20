/*
 * Copyright 2019 Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Extension to client_impl - Refactoring code 
 * Author:          Yuva Athur
 * Modified Date:    Oct. 19. 2020
 * 
 * This refactors original to allow for sub-classing to take care of
 *  + 2 Business Data Columns
 *  + N Business Data Columns
 */


#include "client_impl.h"

#include <algorithm>
#include <iterator>

#include "absl/memory/memory.h"

namespace private_join_and_compute {

PrivateIntersectionSumProtocolClientImpl::
    PrivateIntersectionSumProtocolClientImpl(
        Context* ctx, const std::vector<std::string>& elements,
        const std::vector<BigNum>& values, int32_t modulus_size)
    : ctx_(ctx),
      elements_(elements),
      values_(values),
      p_(ctx_->GenerateSafePrime(modulus_size / 2)),
      q_(ctx_->GenerateSafePrime(modulus_size / 2)),
      intersection_sum_(ctx->Zero()),
      ec_cipher_(std::move(
          ECCommutativeCipher::CreateWithNewKey(
              NID_X9_62_prime256v1, ECCommutativeCipher::HashType::SHA256)
              .value())) {}

StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne>
PrivateIntersectionSumProtocolClientImpl::ReEncryptSet(
    const PrivateIntersectionSumServerMessage::ServerRoundOne& message) {
  private_paillier_ = absl::make_unique<PrivatePaillier>(ctx_, p_, q_, 2);
  BigNum pk = p_ * q_;

  //YAR:: Encrypt the table that Client has
  //    EncryptCol is a virtual call to enable sub-classing
  PrivateIntersectionSumClientMessage::ClientRoundOne result;
  auto maybe_result = EncryptCol();
    if (!maybe_result.ok()) {
      return maybe_result.status();
    }

  result= maybe_result.value();
  *result.mutable_public_key() = pk.ToBytes();


  //YAR:Notes : Reencrypt the ID column Server sent
  std::vector<EncryptedElement> reencrypted_set;
  for (const EncryptedElement& element : message.encrypted_set().elements()) {
    EncryptedElement reencrypted;
    StatusOr<std::string> reenc = ec_cipher_->ReEncrypt(element.element());
    if (!reenc.ok()) {
      return reenc.status();
    }
    *reencrypted.mutable_element() = reenc.value();
    reencrypted_set.push_back(reencrypted);
  }
  std::sort(reencrypted_set.begin(), reencrypted_set.end(),
            [](const EncryptedElement& a, const EncryptedElement& b) {
              return a.element() < b.element();
            });
  for (const EncryptedElement& element : reencrypted_set) {
    *result.mutable_reencrypted_set()->add_elements() = element;
  }

  return result;
}

//YAR::Add : Refactoring
// Original Pair Implementation 
StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> 
PrivateIntersectionSumProtocolClientImpl::EncryptCol(){

  PrivateIntersectionSumClientMessage::ClientRoundOne result;

  for (size_t i = 0; i < elements_.size(); i++) {
    EncryptedElement* element = result.mutable_encrypted_set()->add_elements();
    StatusOr<std::string> encrypted = ec_cipher_->Encrypt(elements_[i]);
    if (!encrypted.ok()) {
      return encrypted.status();
    }
    *element->mutable_element() = encrypted.value();
    StatusOr<BigNum> value = private_paillier_->Encrypt(values_[i]);
    if (!value.ok()) {
      return value.status();
    }
    *element->mutable_associated_data() = value.value().ToBytes();
  }
  return result;
}



//YAR::Add : Refactoring
//This method sets the internal variables
Status PrivateIntersectionSumProtocolClientImpl::DecryptSum(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptSum before ReEncryptSet.");
  }

  StatusOr<BigNum> sum = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum()));
  if (!sum.ok()) {
    return sum.status();
  } 
  intersection_sum_ = sum.value(); //BigNum --> convert to int to print

  return OkStatus();
}


Status PrivateIntersectionSumProtocolClientImpl::StartProtocol(
    MessageSink<ClientMessage>* client_message_sink) {
  ClientMessage client_message;
  *(client_message.mutable_private_intersection_sum_client_message()
        ->mutable_start_protocol_request()) =
      PrivateIntersectionSumClientMessage::StartProtocolRequest();
  return client_message_sink->Send(client_message);
}

Status PrivateIntersectionSumProtocolClientImpl::Handle(
    const ServerMessage& server_message,
    MessageSink<ClientMessage>* client_message_sink) {
  if (protocol_finished()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolClientImpl: Protocol is already "
        "complete.");
  }

  // Check that the message is a PrivateIntersectionSum protocol message.  
  if (!server_message.has_private_intersection_sum_server_message()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolClientImpl: Received a message for the "
        "wrong protocol type");
  }

  if (server_message.private_intersection_sum_server_message()
          .has_server_round_one()) {
    // Handle the server round one message.
    ClientMessage client_message;

    auto maybe_client_round_one =
        ReEncryptSet(server_message.private_intersection_sum_server_message()
                         .server_round_one());
    if (!maybe_client_round_one.ok()) {
      return maybe_client_round_one.status();
    }
    *(client_message.mutable_private_intersection_sum_client_message()
          ->mutable_client_round_one()) =
        std::move(maybe_client_round_one.value());
    return client_message_sink->Send(client_message);
  } else if (server_message.private_intersection_sum_server_message()
                 .has_server_round_two()) {
    // Handle the server round two message.
    auto maybe_result =
        DecryptSum(server_message.private_intersection_sum_server_message()
                       .server_round_two());
    if (!maybe_result.ok()) {
      return maybe_result;
    }

    // Mark the protocol as finished here.
    protocol_finished_ = true;
    return OkStatus();
  }
  // If none of the previous cases matched, we received the wrong kind of
  // message.
  return InvalidArgumentError(
      "PrivateIntersectionSumProtocolClientImpl: Received a server message "
      "of an unknown type.");
}

Status PrivateIntersectionSumProtocolClientImpl::PrintOutput() {
  if (!protocol_finished()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolClientImpl: Not ready to print the "
        "output yet.");
  }

  auto maybe_converted_intersection_sum = intersection_sum_.ToIntValue();
  if (!maybe_converted_intersection_sum.ok()) {
    return maybe_converted_intersection_sum.status();
  }


  std::cout << "Client: The intersection size is " << intersection_size_
            << " and the intersection-sum is "
            << maybe_converted_intersection_sum.value() 
            << std::endl;
  return OkStatus();
}

}  // namespace private_join_and_compute


/*******************************************************************/
/*
//YAR::Add : Refactoring
// Moving the data format specific operations to a helper function
// This will allow for specialization through inheritance  
StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> 
PrivateIntersectionSumProtocolClientImpl::EncryptCol(){
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
*/

/******  Handle Code **************************/
/*     

    auto maybe_result =
        DecryptSum(server_message.private_intersection_sum_server_message()
                       .server_round_two());
    if (!maybe_result.ok()) {
      return maybe_result.status();
    }

    //YAR::Debug: Getting an error when recovering intersection sums
    auto i_size = std::get<0>(maybe_result.value());
    auto i_sum_1 = std::get<1>(maybe_result.value());
    auto i_sum_2 = std::get<2>(maybe_result.value());

    std::cout << "Values got are : Intersection size is " 
          << i_size << "\n"
          << " First encrypted sum is " 
          << i_sum_1.ToIntValue().value()
          << " Second encrypted sum is "
          << i_sum_2.ToIntValue().value();
    //YAR::Edit :Extending to 2 sums
    std::tie(intersection_size_, intersection_sum_1_, intersection_sum_2_) =
        std::move(maybe_result.value());
 */

/*
//YAR::Edit : extending to 2 sums
StatusOr<std::tuple<int64_t, BigNum, BigNum>>
PrivateIntersectionSumProtocolClientImpl::DecryptSum2(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptSum before ReEncryptSet.");
  }

  StatusOr<BigNum> sum_1 = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum_1()));
  if (!sum_1.ok()) {
    return sum_1.status();
  } 

  StatusOr<BigNum> sum_2 = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum_2()));
  if (!sum_2.ok()) {
    return sum_2.status();
  }

  return std::make_tuple(server_message.intersection_size(), sum_1.value(), sum_2.value());
}
*/


/* YAR::Edit : Original for 1 sum
StatusOr<std::pair<int64_t, BigNum>>
PrivateIntersectionSumProtocolClientImpl::DecryptSum(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptSum before ReEncryptSet.");
  }

  StatusOr<BigNum> sum = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum()));
  if (!sum.ok()) {
    return sum.status();
  }
  return std::make_pair(server_message.intersection_size(), sum.value());
}
*/

/**************** ReEncryptSet ************************************/
  /*
  //YAR::Edit : Tuple implementation

  auto ids = std::get<0>(table_);
  auto col_1 = std::get<1>(table_);
  auto col_2 = std::get<2>(table_);

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
  */

  /*
  //YAR::Edit : Pair implementation
  for (size_t i = 0; i < elements_.size(); i++) {
    EncryptedElement* element = result.mutable_encrypted_set()->add_elements();
    StatusOr<std::string> encrypted = ec_cipher_->Encrypt(elements_[i]);
    if (!encrypted.ok()) {
      return encrypted.status();
    }
    *element->mutable_element() = encrypted.value();
    StatusOr<BigNum> value = private_paillier_->Encrypt(values_[i]);
    if (!value.ok()) {
      return value.status();
    }
    *element->mutable_associated_data() = value.value().ToBytes();
  }
 */

/****************** Constructor code *****************************/
/*
//YAR::Edit: Constructor using tuple instead of pair
PrivateIntersectionSumProtocolClientImpl::
    PrivateIntersectionSumProtocolClientImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<BigNum>, std::vector<BigNum>>& table, int32_t modulus_size)
    :ctx_(ctx),
      table_(table),
      p_(ctx_->GenerateSafePrime(modulus_size / 2)),
      q_(ctx_->GenerateSafePrime(modulus_size / 2)),
      intersection_sum_1_(ctx->Zero()),
      intersection_sum_2_(ctx->Zero()),
      ec_cipher_(std::move(
          ECCommutativeCipher::CreateWithNewKey(
              NID_X9_62_prime256v1, ECCommutativeCipher::HashType::SHA256)
              .value())) {}
*/
