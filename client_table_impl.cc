/*
 * Extension to client_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Oct. 16. 2020
 * 
 * ****** Incomplete implementation **************
 * 
 * This extends the Client code to work with dynamic business data columns
 */

#include "client_table_impl.h"

#include <algorithm>
#include <iterator>

#include "absl/memory/memory.h"

namespace private_join_and_compute {

PrivateIntersectionSumProtocolClientTableImpl::PrivateIntersectionSumProtocolClientTableImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<std::vector<BigNum>>>& table, int32_t modulus_size)
    : ctx_(ctx),
      p_(ctx_->GenerateSafePrime(modulus_size / 2)),
      q_(ctx_->GenerateSafePrime(modulus_size / 2)),
      table_(table),
      ec_cipher_(std::move(
          ECCommutativeCipher::CreateWithNewKey(
              NID_X9_62_prime256v1, ECCommutativeCipher::HashType::SHA256)
              .value())),
      intersection_aggregates_(std::get<1>(table).size(),ctx->Zero()) 
      {}

StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne>
PrivateIntersectionSumProtocolClientTableImpl::ReEncryptSet(
    const PrivateIntersectionSumServerMessage::ServerRoundOne& message) {
  private_paillier_ = absl::make_unique<PrivatePaillier>(ctx_, p_, q_, 2);
  BigNum pk = p_ * q_;

  //EncryptCol applies encryption on the business data column
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


StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> 
PrivateIntersectionSumProtocolClientTableImpl::EncryptCol(){

  auto ids = std::get<0>(table_);
  auto col_list = std::get<1>(table_);

  PrivateIntersectionSumClientMessage::ClientRoundOne result;



  // Encrypt one row at a time
  for (size_t i = 0; i < ids.size(); i++) {
    EncryptedElement* element = result.mutable_encrypted_set()->add_elements();
    StatusOr<std::string> encrypted = ec_cipher_->Encrypt(ids[i]);
    if (!encrypted.ok()) {
      return encrypted.status();
    }
    *element->mutable_element() = encrypted.value();
  
    // Encrypt across columns 
    for(size_t j = 0; col_list.size();j++){
      EncryptedElement* dataList = result 
      StatusOr<BigNum> value = private_paillier_->Encrypt(col_list[j][i]);
      if (!value.ok()) {
        return value.status();
      }
      *element->mutable_associated_data_1() = value.value().ToBytes();
    }

    //YAR::Note : This is where the business keys are encrypted using homomorphic encryption

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
//This method sets the internal variables
Status PrivateIntersectionSumProtocolClientTableImpl::DecryptResult(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptResult before ReEncryptSet.");
  }

  intersection_size_ = server_message.intersection_size();        

  StatusOr<BigNum> sum = private_paillier_->Decrypt(
      ctx_->CreateBigNum(server_message.encrypted_sum()));
  if (!sum.ok()) {
    return sum.status();
  } 
  intersection_sum_ = sum.value(); //BigNum --> convert to int to print

  return OkStatus();
}


Status PrivateIntersectionSumProtocolClientTableImpl::StartProtocol(
    MessageSink<ClientMessage>* client_message_sink) {
  ClientMessage client_message;
  *(client_message.mutable_private_intersection_sum_client_message()
        ->mutable_start_protocol_request()) =
      PrivateIntersectionSumClientMessage::StartProtocolRequest();
  return client_message_sink->Send(client_message);
}

Status PrivateIntersectionSumProtocolClientTableImpl::Handle(
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
        DecryptResult(server_message.private_intersection_sum_server_message()
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

Status PrivateIntersectionSumProtocolClientTableImpl::PrintOutput() {
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
PrivateIntersectionSumProtocolClientTableImpl::EncryptCol(){
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
Status PrivateIntersectionSumProtocolClientTableImpl::DecryptResult(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message) {
  if (private_paillier_ == nullptr) {
    return InvalidArgumentError("Called DecryptSum before ReEncryptSet.");
  }

  intersection_size_ = server_message.intersection_size();        


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



Status PrivateIntersectionSumProtocolClientTableImpl::PrintOutput() {
  if (!protocol_finished()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolClientImpl: Not ready to print the "
        "output yet.");
  }
  //YAR::Edit : Got vector of aggregates
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

*/