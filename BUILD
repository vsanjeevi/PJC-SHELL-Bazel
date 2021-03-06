load("@com_github_grpc_grpc//bazel:grpc_build_system.bzl", "grpc_proto_library")

grpc_proto_library(
    name = "match_proto",
    srcs = ["match.proto"],
)

grpc_proto_library(
    name = "private_intersection_sum_proto",
    srcs = ["private_intersection_sum.proto"],
    deps = [
        ":match_proto",
    ],
)

grpc_proto_library(
    name = "private_join_and_compute_proto",
    srcs = ["private_join_and_compute.proto"],
    deps = [
        ":private_intersection_sum_proto",
    ],
)

cc_library(
    name = "message_sink",
    hdrs = ["message_sink.h"],
    deps = [
        ":private_join_and_compute_proto",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

cc_library(
    name = "protocol_client",
    hdrs = ["protocol_client.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        "//util:status_includes",
    ],
)

# client_impl : Uses Pair Constructor (Original Code) <id, col>
cc_library(
    name = "client_impl",
    srcs = ["client_impl.cc"],
    hdrs = ["client_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

# client_tuple_impl : Extends client_impl to use tuple <id, col1, col2>
cc_library(
    name = "client_tuple_impl",
    srcs = ["client_tuple_impl.cc"],
    hdrs = ["client_tuple_impl.h"],
    deps = [
        ":client_impl",
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)


cc_library(
    name = "protocol_server",
    hdrs = ["protocol_server.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        "//util:status_includes",
    ],
)

cc_library(
    name = "server_impl",
    srcs = ["server_impl.cc"],
    hdrs = ["server_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

# server_tuple_impl : Extends server_impl to use tuple <id, col1, col2>
cc_library(
    name = "server_tuple_impl",
    srcs = ["server_tuple_impl.cc"],
    hdrs = ["server_tuple_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

cc_library(
    name = "data_util",
    srcs = ["data_util.cc"],
    hdrs = ["data_util.h"],
    deps = [
        ":match_proto",
        "//crypto:bn_util",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "generate_dummy_data",
    srcs = ["generate_dummy_data.cc"],
    deps = [
        ":data_util",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/base",
    ],
)

cc_library(
    name = "private_join_and_compute_rpc_impl",
    srcs = ["private_join_and_compute_rpc_impl.cc"],
    hdrs = ["private_join_and_compute_rpc_impl.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//util:status_includes",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "server",
    srcs = ["server.cc"],
    deps = [
        ":data_util",
        ":private_join_and_compute_proto",
        ":private_join_and_compute_rpc_impl",
        ":protocol_server",
        ":server_impl",
        ":server_tuple_impl",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/memory",
    ],
)

cc_binary(
    name = "client",
    srcs = ["client.cc"],
    deps = [
        ":client_impl",
        ":client_tuple_impl",
        ":data_util",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)



# ************************************************************
# from google shell


load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

# Protos.
proto_library(
    name = "serialization_proto",
    srcs = ["serialization.proto"],
)

cc_proto_library(
    name = "serialization_cc_proto",
    deps = [":serialization_proto"],
)

# Context.

cc_library(
    name = "context",
    hdrs = ["context.h"],
    deps = [
        ":error_params",
        ":ntt_parameters",
        ":statusor_fork",
        "@com_google_absl//absl/memory",
    ],
)

cc_test(
    name = "context_test",
    size = "small",
    srcs = ["context_test.cc"],
    deps = [
        ":constants",
        ":context",
        ":integral_types",
        ":montgomery",
        ":statusor_fork",
        "//testing:parameters",
        "//testing:status_testing",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)

# Integral types for integers.

cc_library(
    name = "integral_types",
    hdrs = ["integral_types.h"],
    deps = [
        "@com_google_absl//absl/numeric:int128",
    ],
)

# Constants.

cc_library(
    name = "constants",
    hdrs = ["constants.h"],
    deps = [
        ":integral_types",
        "@com_google_absl//absl/numeric:int128",
    ],
)

# Utilities.

cc_library(
    name = "bits_util",
    hdrs = ["bits_util.h"],
    deps = [
        ":integral_types",
        "@com_google_absl//absl/numeric:int128",
    ],
)

cc_test(
    name = "bits_util_test",
    size = "small",
    srcs = ["bits_util_test.cc"],
    deps = [
        ":bits_util",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)

cc_library(
    name = "sample_error",
    hdrs = ["sample_error.h"],
    deps = [
        ":bits_util",
        ":constants",
        ":error_params",
        ":statusor_fork",
        "//prng",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "sample_error_test",
    size = "small",
    srcs = [
        "sample_error_test.cc",
    ],
    deps = [
        ":context",
        ":montgomery",
        ":sample_error",
        ":symmetric_encryption",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "@com_github_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "statusor_fork",
    srcs = ["statusor.cc"],
    hdrs = [
        "status_macros.h",
        "statusor.h",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "statusor_test",
    size = "small",
    srcs = ["statusor_test.cc"],
    deps = [
        ":statusor_fork",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status",
    ],
)

cc_test(
    name = "status_macros_test",
    size = "small",
    srcs = ["status_macros_test.cc"],
    deps = [
        ":statusor_fork",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/status",
    ],
)

# Montgomery integers.

cc_library(
    name = "montgomery",
    srcs = ["montgomery.cc"],
    hdrs = ["montgomery.h"],
    deps = [
        ":bits_util",
        ":constants",
        ":int256",
        ":serialization_cc_proto",
        ":statusor_fork",
        ":transcription",
        "//prng",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/numeric:int128",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "montgomery_test",
    size = "small",
    srcs = [
        "montgomery_test.cc",
    ],
    deps = [
        ":constants",
        ":montgomery",
        ":serialization_cc_proto",
        ":statusor_fork",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)

# NTT parameters.

cc_library(
    name = "ntt_parameters",
    srcs = ["ntt_parameters.cc"],
    hdrs = ["ntt_parameters.h"],
    deps = [
        ":constants",
        ":statusor_fork",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "ntt_parameters_test",
    size = "small",
    srcs = [
        "ntt_parameters_test.cc",
    ],
    deps = [
        ":constants",
        ":montgomery",
        ":ntt_parameters",
        ":statusor_fork",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)

# NTT polynomial.

cc_library(
    name = "polynomial",
    hdrs = ["polynomial.h"],
    deps = [
        ":constants",
        ":ntt_parameters",
        ":serialization_cc_proto",
        ":statusor_fork",
        "//prng",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "polynomial_test",
    size = "small",
    srcs = [
        "polynomial_test.cc",
    ],
    deps = [
        ":constants",
        ":montgomery",
        ":ntt_parameters",
        ":polynomial",
        ":serialization_cc_proto",
        ":statusor_fork",
        "//prng:integral_prng_testing_types",
        "//testing:coefficient_polynomial",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "@com_github_google_googletest//:gtest_main",
    ],
)

# Error parameters.

cc_library(
    name = "error_params",
    hdrs = ["error_params.h"],
    deps = [
        ":montgomery",
        ":ntt_parameters",
        ":statusor_fork",
    ],
)

cc_test(
    name = "error_params_test",
    size = "small",
    srcs = [
        "error_params_test.cc",
    ],
    deps = [
        ":constants",
        ":context",
        ":error_params",
        ":montgomery",
        ":ntt_parameters",
        ":statusor_fork",
        ":symmetric_encryption",
        "//prng:integral_prng_types",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "//testing:testing_utils",
        "@com_github_google_googletest//:gtest_main",
    ],
)

# Encryption.

cc_library(
    name = "symmetric_encryption",
    hdrs = ["symmetric_encryption.h"],
    deps = [
        ":error_params",
        ":polynomial",
        ":sample_error",
        ":serialization_cc_proto",
        ":statusor_fork",
        "//prng",
        "//prng:integral_prng_types",
    ],
)

cc_test(
    name = "symmetric_encryption_test",
    size = "medium",
    srcs = [
        "symmetric_encryption_test.cc",
    ],
    deps = [
        ":constants",
        ":context",
        ":montgomery",
        ":ntt_parameters",
        ":polynomial",
        ":serialization_cc_proto",
        ":statusor_fork",
        ":symmetric_encryption",
        "//prng:integral_prng_types",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "//testing:testing_utils",
        "@com_github_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "symmetric_encryption_with_prng",
    hdrs = ["symmetric_encryption_with_prng.h"],
    deps = [
        ":polynomial",
        ":statusor_fork",
        ":symmetric_encryption",
        "//prng",
        "//prng:integral_prng_types",
    ],
)

cc_test(
    name = "symmetric_encryption_with_prng_test",
    size = "small",
    srcs = [
        "symmetric_encryption_with_prng_test.cc",
    ],
    deps = [
        ":context",
        ":montgomery",
        ":ntt_parameters",
        ":polynomial",
        ":statusor_fork",
        ":symmetric_encryption_with_prng",
        "//prng:integral_prng_types",
        "//testing:parameters",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_utils",
        "@com_github_google_googletest//:gtest_main",
    ],
)

# Relinearization Key

cc_library(
    name = "relinearization_key",
    srcs = ["relinearization_key.cc"],
    hdrs = ["relinearization_key.h"],
    deps = [
        ":bits_util",
        ":montgomery",
        ":sample_error",
        ":statusor_fork",
        ":symmetric_encryption",
        ":symmetric_encryption_with_prng",
        "//prng:integral_prng_types",
        "@com_google_absl//absl/numeric:int128",
    ],
)

cc_test(
    name = "relinearization_key_test",
    size = "small",
    srcs = ["relinearization_key_test.cc"],
    deps = [
        ":constants",
        ":montgomery",
        ":ntt_parameters",
        ":polynomial",
        ":relinearization_key",
        ":statusor_fork",
        ":symmetric_encryption",
        "//prng:integral_prng_types",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "@com_github_google_googletest//:gtest_main",
    ],
)

# Galois Key.

cc_library(
    name = "galois_key",
    hdrs = ["galois_key.h"],
    deps = [
        ":relinearization_key",
        ":statusor_fork",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "galois_key_test",
    size = "small",
    srcs = ["galois_key_test.cc"],
    deps = [
        ":constants",
        ":galois_key",
        ":montgomery",
        ":ntt_parameters",
        ":polynomial",
        ":statusor_fork",
        ":symmetric_encryption",
        "//prng:integral_prng_types",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "//testing:testing_prng",
        "//testing:testing_utils",
        "@com_github_google_googletest//:gtest_main",
    ],
)

# int256 Type

cc_library(
    name = "int256",
    srcs = ["int256.cc"],
    hdrs = ["int256.h"],
    deps = [
        ":integral_types",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/numeric:int128",
    ],
)

cc_test(
    name = "int256_test",
    size = "small",
    srcs = ["int256_test.cc"],
    deps = [
        ":int256",
        "@com_github_google_glog//:glog",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/container:fixed_array",
        "@com_google_absl//absl/numeric:int128",
    ],
)

# Transcription

cc_library(
    name = "transcription",
    hdrs = ["transcription.h"],
    deps = [
        ":statusor_fork",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "transcription_test",
    size = "small",
    srcs = ["transcription_test.cc"],
    deps = [
        ":integral_types",
        ":statusor_fork",
        ":transcription",
        "//testing:status_is_fork",
        "//testing:status_testing",
        "@com_github_google_googletest//:gtest_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)
