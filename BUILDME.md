# Understanding Bazel Build Server

Ref: http://gensoft.pasteur.fr/docs/bazel/0.3.0/bazel-user-manual.html 
+ Client / Server architecture
    + Client starts a Server if no running Server found
    + Server discovered using
        + base workspace directory and 
        + your userid.

## Phases of a Build
+ **Load** : Loads necessary file & computes transitivie dependencies
    + Errors found include inappropriate dependencies, invalid inputs to a rule, and all rule-specific error messages.
    + Before you can start a build, you will need a Bazel workspace. 
        + This is simply a directory tree that contains all the source files needed to build your application. 
        + Bazel allows you to perform a build from a completely read-only volume.
+ **Analysis** : Semantic analysis and validation of each build rule, the construction of a build dependency graph, and the determination of exactly what work is to be done in each step of the build.
    + Errors reported at this stage include: inappropriate dependencies, invalid inputs to a rule, and all rule-specific error messages.
+ **Execution** : Ensures that the outputs of each step in the build are consistent with its inputs, re-running compilation/linking/etc. tools as necessary.
    + Errors reported during this phase include: missing source files, errors in a tool executed by some build action, or failure of a tool to produce the expected set of outputs.

## Understanding Bazel build for C++
Ref: https://docs.bazel.build/versions/master/tutorial/cpp.html 
+ Build files
    + *Workspace file* : 
        + Root Folder
        + Identifies root and sub-folders as project's directory structure
    + *Build file(s)* :
        + Instructions to build a project
        + *Package* : A subfolder that contains a build file
+ Query options
    + Ref: https://docs.bazel.build/versions/master/query-how-to.html 
    + bazel query --noimplicit_deps --incompatible_disable_deprecated_attr_params=false "deps(//:generate_dummy_data)" --output graph > dep_gen_dummy_data.txt

    + bazel query --noimplicit_deps --incompatible_disable_deprecated_attr_params=false "deps(//:match_proto)" --output graph > dep_match_proto.txt

    + bazel query "deps(//:private_intersection_sum_proto)" --output graph > dep_pis_proto.txt

    + bazel query "deps(//:private_join_and_compute_proto)" --output graph > dep_pjc_proto.txt
    
    + bazel query --noimplicit_deps "deps(//:protocol_client)" --output graph > dep_protocol_client.txt

    + which rules are defined in package root?
    + + bazel query 'kind(rule, //:*)' --output label_kind


## https://docs.bazel.build/versions/master/query.html
    + bazel query --noimplicit_deps "deps(//:protocol_client,3)" --output graph > dep_protocol_client_3.txt

    + bazel query --noimplicit_deps "deps(//:client,4)" --output graph > dep_client_4.txt

## Previous explorations
    + Graph view at : http://www.webgraphviz.com/
    + Local vizualization : 
        + sudo apt install graphviz xdot
        + xdot viewing says unexpected end of file
    + xdot <(bazel query --notool_deps --noimplicit_deps "deps(//:match_proto)" --output graph)
        + does not work!
    + bazel query "deps(//:match_proto)" --incompatible_disable_deprecated_attr_params=false
    + bazel query 'buildfiles(deps(//:match_proto))' --output package --incompatible_disable_deprecated_attr_params=false
    + bazel query 'kind(match_proto,deps(//:match_proto))' --output package --incompatible_disable_deprecated_attr_params=false

+ Exploring build sequences
    + bazel build //:match_proto --incompatible_disable_deprecated_attr_params=false --incompatible_depset_is_not_iterable=false --incompatible_new_actions_api=false --incompatible_no_support_tools_in_action_inputs=false
+ Exploring dependency visualization
    + https://docs.bazel.build/versions/master/query-how-to.html 
    + bazel query --noimplicit_deps --incompatible_disable_deprecated_attr_params=false "deps(//:match_proto)" --output graph > graph
    + Graph view at : http://www.webgraphviz.com/
    
## External libraries used in Private-Join-and-Compute
+ name = "com_google_protobuf",
    + remote = "https://github.com/google/protobuf.git",    
+ name = "com_github_glog_glog",
    + urls = ["https://github.com/google/glog/archive/v0.3.5.tar.gz"],

+ name = "com_github_gflags_gflags",
    + remote = "https://github.com/gflags/gflags.git",
    + gflags: Needed for glog


+ name = "boringssl",
    + "https://github.com/google/boringssl/archive/a0fb951d2a26a8ee746b52f3ba81ab011a0af778.tar.gz",

+ name = "com_google_absl",
    + remote = "https://github.com/abseil/abseil-cpp.git",
    + Abseil C++ libraries : Extends STL (from Google)

+ name = "com_github_grpc_grpc",
    + remote = "https://github.com/grpc/grpc.git",

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

# Debugging in VSCode
+ Ref: https://shanee.io/blog/2019/05/28/bazel-with-visual-studio-code/ 

# Syncing from Upstream


https://wiki.paparazziuav.org/wiki/Github_manual_for_Ubuntu 
https://www.youtube.com/watch?v=-zvHQXnBO6c Syncing Your GitHub Fork

# Protobuff 2
Ref: https://developers.google.com/protocol-buffers/docs/proto 


# GitHub sync from Google Master
https://wiki.paparazziuav.org/wiki/Github_manual_for_Ubuntu 
https://www.youtube.com/watch?v=-zvHQXnBO6c Syncing Your GitHub Fork