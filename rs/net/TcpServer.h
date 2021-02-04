#pragma once

#include "noncopyable.h"
#include "ServerBase.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net//EventLoop.h>

namespace rs{

class TcpServer : ServerBase{
private:
  muduo::net::TcpServer _tcp_server;
public:
  virtual void start(uint16_t port){
    muduo::net::EventLoop loop;
//     uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    muduo::net::InetAddress serverAddr(port);
    this->_tcp_server(&loop, serverAddr);
//     server.start();
    loop.loop();
    this->_tcp_server.
  }

  virtual void stop() {

  }

  void bind(std::string ip, std::size_t port){

  }

  void listen(std::size_t backlog){

  }


 };

} // namespace rs

