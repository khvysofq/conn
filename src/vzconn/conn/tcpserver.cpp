#include "vzconn/conn/tcpserver.h"

namespace vzconn {

  TcpServer::Ptr TcpServer::CreateTcpServer(boost::asio::io_service& io_service,
    const std::string addr, uint16 port){

    boost::asio::ip::tcp::endpoint bind_addr(
      boost::asio::ip::address().from_string(addr), port);

    return TcpServer::Ptr(new TcpServer(io_service, bind_addr));
  }

  TcpServer::TcpServer(boost::asio::io_service& io_service,
    const boost::asio::ip::tcp::endpoint& bind_addr)
    :io_service_(io_service),
    bind_addr_(bind_addr),
    stop_(true){

  }

  TcpServer::~TcpServer(){
    SignalNewTcpConnect.disconnect_all_slots();
    SignalTcpServerError.disconnect_all_slots();
  }

  bool TcpServer::Start(){
    if (!stop_){
      //LOG(ERROR) << "The TcpServer is not stop, you should not Start agian";
      return false;
    }
    try{
      acceptor_.reset(new boost::asio::ip::tcp::acceptor(io_service_, bind_addr_.protocol()));
      acceptor_->bind(bind_addr_);
      acceptor_->listen();
    }
    catch (std::exception& e){
      std::cout << e.what() << std::endl;
      //LOG(ERROR) << e.what();
      return false;
    }
    stop_ = false;
    io_service_.post(
      boost::bind(&TcpServer::HandleStartAcceptor, shared_from_this()));
    return true;
  }

  void TcpServer::Stop(){
    if (stop_){
      return;
    }
    else{
      // Fast stop this server
      stop_ = true;
      io_service_.post(
        boost::bind(&TcpServer::HandleStopTcpServer, shared_from_this()));
    }
  }

  void TcpServer::HandleStartAcceptor(){
    if (stop_){
      //LOG(WARNING) << "The TcpServer was stoped";
      return;
    }
    if (!acceptor_){
      //LOG(ERROR) << "The Acceptor is null";
      return;
    }
    TcpConnect::Ptr connect(new TcpConnect(io_service_));
    acceptor_->async_accept(connect->socket(),
      boost::bind(&TcpServer::HandleAcceptor, shared_from_this(),
      connect, boost::asio::placeholders::error));
  }

  void TcpServer::HandleAcceptor(TcpConnect::Ptr new_connect,
    const boost::system::error_code& err){

    if (stop_){
      //LOG(WARNING) << "The TcpServer was stoped";
      return;
    }

    if (err){
      stop_ = true;
      //LOG(ERROR) << err.message();
      SignalTcpServerError(shared_from_this(), err);
      return;
    }
    SignalNewTcpConnect(shared_from_this(), new_connect);
    new_connect->StartReadData();
    HandleStartAcceptor();
  }

  void TcpServer::HandleStopTcpServer(){
    if (acceptor_){
      acceptor_->close();
    }
  }
}