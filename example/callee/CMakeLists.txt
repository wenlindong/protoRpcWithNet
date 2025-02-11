set(SRC_LIST Friendservice.cc ../Friend.pb.cc)

add_executable(provider ${SRC_LIST})  #生成provider可执行文件
target_link_libraries(provider protoRpc protobuf)