#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include "Rpcheader.pb.h"
#include "Rpcchannel.h"
#include "Rpcapplication.h"
#include "Rpccontroller.h"

/*
header_size + service_name method_name args_size + args
*/
// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送 
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf:: Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name(); // service_name
    std::string method_name = method->name(); // method_name

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    ///从request序列化数据到字符串args_str中
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        return;
    }
    
    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送的rpc请求的字符串
    ///发送的字符串构成：头部信息字符串的长度（int32转成字符串） + 头部信息字符串 + 参数字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str; // rpcheader
    send_rpc_str += args_str; // args

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str_size: " << args_str.size() << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 读取配置文件rpcserver的信息
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};  ///TODO：空间不一定够
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    ///////////////////////////////////////上面 接收rpc请求响应值代码的优化，保证读到数据的完整性
    // 接收 rpc 请求的响应值
    /*uint32_t expected_response_size = 0;  // 期望的响应数据长度
    // 先接收头部，头部包含响应数据的长度
    char recv_buf[4];
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 4, 0))) {
        close(clientfd);
        char errtxt[512] = { 0 };
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    std::memcpy(&expected_response_size, recv_buf, 4);
    expected_response_size = ntohl(expected_response_size);  // 网络字节序转为主机字节序

    std::string response_str;
    response_str.resize(expected_response_size);
    size_t total_received = 0;
    while (total_received < expected_response_size) {
        int n = recv(clientfd, &response_str[total_received], expected_response_size - total_received, 0);
        if (n == -1) {
            close(clientfd);
            char errtxt[512] = { 0 };
            sprintf(errtxt, "recv error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }
        if (n == 0) {  // 连接关闭
            close(clientfd);
            controller->SetFailed("Connection closed prematurely.");
            return;
        }
        total_received += n;
    }*/
    //在接收响应时，先接收一个 4 字节的头部，这个头部存储了响应数据的长度（使用网络字节序）。
    //使用 ntohl 将网络字节序转换为主机字节序，得到 expected_response_size。
    //然后，根据 expected_response_size 调整 response_str 的大小，并使用一个循环不断接收数据，直到接收到完整的数据。
    //循环中，使用 recv 接收数据，并更新 total_received，直到接收的数据长度达到 expected_response_size。
    /////////////////////////////////////////////////////优化结束

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size); // bug出现问题，recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败
    // if (!response->ParseFromString(response_str))
    ///从recv_buf反序列化数据到response
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char errtxt[512] = {0}; ///to do:空间太小了，放不下recv_buf
        sprintf(errtxt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);
}