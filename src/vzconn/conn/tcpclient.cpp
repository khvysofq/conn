#include "vzconn/conn/tcpclient.h"

namespace vzconn{

  TcpClient::Ptr TcpClient::CreateTcpClient(boost::asio::io_service& io_service,
    const std::string addr, uint16 port){

    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    return TcpClient::Ptr(new TcpClient(io_service, server_addr));
  }

  TcpConnect::Ptr TcpClient::ConnectServer(boost::asio::io_service& io_service,
    const std::string addr, uint16 port, CallBackConnect call_back){

    TcpConnect::Ptr connect(new TcpConnect(io_service));
    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    connect->socket().async_connect(server_addr,
      boost::bind(&TcpClient::HandleStaticServerConnect, connect,
      boost::asio::placeholders::error, call_back));
    return connect;
  }

  void TcpClient::HandleStaticServerConnect(TcpConnect::Ptr connect,
    const boost::system::error_code& err, CallBackConnect call_back){
    if (err){
      call_back(connect, err);
    }
    else{
      connect->StartReadData();
      call_back(connect, err);
    }
  }

  TcpClient::TcpClient(boost::asio::io_service& io_service,
    const boost::asio::ip::tcp::endpoint& addr)
    :io_service_(io_service),
    server_addr_(addr){

  }

  TcpClient::~TcpClient(){
    SignalConnect.disconnect_all_slots();
  }

  TcpConnect::Ptr TcpClient::Connect(){

    TcpConnect::Ptr connect(new TcpConnect(io_service_));

    connect->socket().async_connect(server_addr_,
      boost::bind(&TcpClient::HandleServerConnect, shared_from_this(),
      connect,
      boost::asio::placeholders::error));
    return connect;
  }

  void TcpClient::HandleServerConnect(TcpConnect::Ptr connect,
    const boost::system::error_code& err){
    SignalConnect(shared_from_this(), connect, err);
    connect->StartReadData();
  }

}