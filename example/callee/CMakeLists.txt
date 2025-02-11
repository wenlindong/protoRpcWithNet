set(SRC_LIST Friendservice.cc ../Friend.pb.cc)

add_executable(provider ${SRC_LIST})
target_link_libraries(provider protoRpc protobuf)