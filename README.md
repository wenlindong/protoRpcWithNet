# protoRpcWithNet

### 一. 介绍

本项目是在Linux平台下使用C++11开发的分布式网络通信（RPC）框架。由两部分组成：其一是基于epoll非阻塞IO多路复用技术开发的并发网络库；其二是基于该网络库和Protobuf库开发的RPC框架。通过该框架可以实现跨机台远程调用服务。

### 二. 主要内容

项目主要包括以下几个方面：

- 基于epoll非阻塞IO多路复用技术（Reactor模式）+ 多线程实现了并发网络库；
- 网络库使用多线程来充分利用多核CPU，并构建线程池来避免线程频繁创建销毁开销；
- 线程池模型采用每个线程一个事件循环的方式，同一连接中产生的事件放到同一程中处理来增加亲和性；
- 网络库基于事件驱动的方式运转，采用function+bind的方式实现事件回调，并计事件分发器来响应不同类型的事件；
- 网络库中设计缓冲器来保证收发数据的完整性，缓冲器支持自动增长和手动收缩。
- Rpc服务中设计编解码器来处理消息和网络字节流的转换；
- Rpc服务中使用protobuf库对消息来进行序列化和反序列化，利用protoc编译生的类元信息通过反射机制实现自动创建具体类型的消息对象；
- 系统采用分层设计，将网络IO部分和业务部分（rpc服务）进行了解耦；
- 系统使用智能指针自动管理动态对象的生命周期，增加资源管理方面的健壮性。

### 三. 项目环境配置

本项目依赖了第三方库Protobuf，环境配置如下：

   - **ubuntu下 protobuf**环境搭建
     - 在**github**源代码下载地址：[https://github.com/google/protobuf](https://github.com/google/protobuf)，源码包中的src/README.md里有详细的安装说明，安装过程如下：

   ```tex
   1、解压压缩包：unzip protobuf-master.zip
   2、进入解压后的文件夹：cd protobuf-master
   3、安装所需工具：sudo apt-get install autoconf automake libtool curl make g++ unzip
   4、自动生成confifigure配置文件：./autogen.sh
   5、配置环境：./configure 
   6、编译源代码：make
   7、安装：sudo make install
   8、刷新动态库：sudo ldconfig
   ```

### 四. 项目目录结构

```tex
bin：存放可执行文件，具体放的是example编译好的二进制文件
build：项目构建文件夹
lib：编译完成的RPC库文件和需要被引用的头文件
src：RPC源文件
src/net：网络库源文件
example：RPC框架代码使用示例
CMakeLists.txt：顶层的cmake文件
README.md：项目自述文件
autobuild.sh：一键编译脚本
```

- `autobuild.sh`是项目的一键构建脚本，最终提供给用户的是静态库文件和头文件，都放在lib下面
  - `libprotoRpc.a`：rpc库文件
  - `include`：头文件目录
