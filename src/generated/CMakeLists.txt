file(GLOB_RECURSE PROTOS
    ${CMAKE_SOURCE_DIR}/include/satpos/api/v1/*.proto
)

# message(${PROTOS})

add_library(propsrv STATIC)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/api/v1")
set(PROTO_IMPORT_DIRS "${CMAKE_SOURCE_DIR}/include/satpos/api/v1")

# message(${PROTO_BINARY_DIR})
# message(${PROTO_IMPORT_DIRS})

# print_common_cmake_vars()

protobuf_generate(
    TARGET propsrv
    LANGUAGE cpp
    PROTOS ${PROTOS}
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

protobuf_generate(
    TARGET propsrv
    LANGUAGE grpc
    PROTOS ${PROTOS}
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)


target_include_directories(propsrv 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(propsrv
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
)

