#include "proxy_client/fake/fakeclient.h"

namespace proxy{

  // ---------------------------------------------------------------------------
  FakeClient::Ptr FakeClient::CreateFakeClient(boost::asio::io_service& io_service,
    const std::string addr, uint16 port){

    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    return FakeClient::Ptr(new FakeClient(io_service, server_addr));
  }

  FakeClientConn::Ptr FakeClient::ConnectServer(boost::asio::io_service& io_service,
    const std::string addr, uint16 port, CallBackConnect call_back){

    FakeClientConn::Ptr connect(new FakeClientConn(io_service));

    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    connect->Socket().async_connect(server_addr,
      boost::bind(&FakeClient::HandleStaticServerConnect, connect,
      boost::asio::placeholders::error, call_back));
    return connect;
  }

  void FakeClient::HandleStaticServerConnect(FakeClientConn::Ptr connect,
    const boost::system::error_code& err, CallBackConnect call_back){
    call_back(connect, err);
  }

  FakeClient::FakeClient(boost::asio::io_service& io_service,
    const boost::asio::ip::tcp::endpoint& addr)
    :io_service_(io_service),
    server_addr_(addr){

  }

  FakeClient::~FakeClient(){
  }
}