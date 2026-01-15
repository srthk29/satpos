# https://github.com/protocolbuffers/protobuf/blob/main/docs/cmake_protobuf_generate.md

# done at root level
# find_package(protobuf CONFIG REQUIRED)
# find_package(gRPC CONFIG REQUIRED)

# file(GLOB_RECURSE PROTOS "${CMAKE_CURRENT_LIST_DIR}/api/v1/sat.proto")

# add_library(proto-objects OBJECT "${CMAKE_CURRENT_LIST_DIR}/api/v1/sat.proto")
# add_library(proto-objects STATIC)
add_library(proto-objects STATIC "${CMAKE_CURRENT_LIST_DIR}/api/v1/sat.proto")

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}")

# target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
target_include_directories(proto-objects PUBLIC "${PROTO_BINARY_DIR}")

# message(${PROTO_BINARY_DIR})
# message(${PROTO_IMPORT_DIRS})

# print_common_cmake_vars()

protobuf_generate(
    TARGET proto-objects
    LANGUAGE cpp
    # PROTOS ${PROTOS}
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

protobuf_generate(
    TARGET proto-objects
    LANGUAGE grpc
    # PROTOS ${PROTOS}
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

