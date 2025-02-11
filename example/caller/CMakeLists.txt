set(SRC_LIST Callfriendservice.cc ../Friend.pb.cc)

add_executable(consumer ${SRC_LIST})  #生成consumer可执行文件
target_link_libraries(consumer simpleRpc protobuf)