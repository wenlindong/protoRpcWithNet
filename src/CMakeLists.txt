set(SRC_DIR ".")  # 设置源文件目录
file(GLOB_RECURSE SRC_LIST "${SRC_DIR}/*.cc")  # 递归获取目录下所有.cc文件

add_library(protoRpc ${SRC_LIST}) #生成protoRpc静态库
target_link_libraries(protoRpc pthread)