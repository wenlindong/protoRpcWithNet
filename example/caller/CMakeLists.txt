set(SRC_LIST Callfriendservice.cc ../Friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer simpleRpc protobuf)