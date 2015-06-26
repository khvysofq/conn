#include "proxy_client/conn/proxy_server.h"
#include "proxy_client/conn/httpconnpair.h"

namespace proxy {

  TcpServer::Ptr TcpServer::CreateTcpServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const std::string addr, uint16 port){

    // LOG(INFO) << addr << ":" << port;

    boost::asio::ip::tcp::endpoint bind_addr(
      boost::asio::ip::address().from_string(addr), port);

    return TcpServer::Ptr(new TcpServer(io_services, accpet_service, bind_addr));
  }

  TcpServer::TcpServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const boost::asio::ip::tcp::endpoint& bind_addr)
    :io_services_(io_services),
    accpet_service_(accpet_service),
    bind_addr_(bind_addr),
    stop_(true){

  }

  TcpServer::~TcpServer(){
    SignalNewTcpConnect.disconnect_all_slots();
    SignalTcpServerError.disconnect_all_slots();
  }

  bool TcpServer::Start(){
    if (!stop_){
      // LOG(ERROR) << "The TcpServer is not stop, you should not Start agian";
      return false;
    }
    try{
      acceptor_.reset(new boost::asio::ip::tcp::acceptor(accpet_service_, bind_addr_.protocol()));
      acceptor_->bind(bind_addr_);
      acceptor_->listen();
    }
    catch (std::exception& e){
      std::cout << e.what() << std::endl;
      // LOG(ERROR) << e.what();
      return false;
    }
    stop_ = false;
    accpet_service_.post(
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
      accpet_service_.post(
        boost::bind(&TcpServer::HandleStopTcpServer, shared_from_this()));
    }
  }

  void TcpServer::HandleStartAcceptor(){
    if (stop_){
      // LOG(WARNING) << "The TcpServer was stoped";
      return;
    }
    if (!acceptor_){
      // LOG(ERROR) << "The Acceptor is null";
      return;
    }
    HttpConn::Ptr connect(HttpConn::CreateHttpConn(get_best_io_server()));
    acceptor_->async_accept(connect->socket(),
      boost::bind(&TcpServer::HandleAcceptor, shared_from_this(),
      connect, boost::asio::placeholders::error));
  }

  void TcpServer::HandleAcceptor(HttpConn::Ptr connect,
    const boost::system::error_code& err){

    if (stop_){
      // LOG(WARNING) << "The TcpServer was stoped";
      return;
    }

    if (err){
      stop_ = true;
      // LOG(ERROR) << err.message();
      return;
    }
    //SignalNewTcpConnect(shared_from_this(), connect);
    connect->StartHttpConn(
      boost::bind(&TcpServer::HandleHttpMethodComplete, shared_from_this(), _1, _2));
    HandleStartAcceptor();
  }

  void TcpServer::HandleStopTcpServer(){
    if (acceptor_){
      acceptor_->close();
    }
  }

  void TcpServer::HandleHttpMethodComplete(HttpConn::Ptr conn, Request& req){
    // DLOG(INFO) << req.method << " : " << req.host << ":" << req.port 
   //   << " " << req.uri;
    HttpConnPair::Ptr conn_pair(new HttpConnPair(conn));
    conn_pair->Start();
  }

  boost::asio::io_service& TcpServer::get_best_io_server(){
    io_services_.push_back(io_services_.front());
    io_services_.pop_front();
    return *io_services_.front();
  }
} // namespace proxy