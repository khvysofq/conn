#include "proxy_client/fake/fakeclientserver.h"
#include "proxy_client/fake/fakeclientconnsession.h"

namespace proxy {

  FakeClientServer::Ptr FakeClientServer::CreateFakeClientServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const std::string addr, uint16 port, const std::string fake_server, unsigned short fake_port){

    // LOG(INFO) << addr << ":" << port;

    boost::asio::ip::tcp::endpoint bind_addr(
      boost::asio::ip::address().from_string(addr), port);

    return FakeClientServer::Ptr(new FakeClientServer(
      io_services, accpet_service, bind_addr, fake_server, fake_port));
  }

  FakeClientServer::FakeClientServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const boost::asio::ip::tcp::endpoint& bind_addr,
    const std::string fake_server, unsigned short fake_port)
    :io_services_(io_services),
    accpet_service_(accpet_service),
    bind_addr_(bind_addr),
    stop_(true),
    fake_server_(fake_server),
    fake_port_(fake_port){

  }

  FakeClientServer::~FakeClientServer(){
    SignalNewTcpConnect.disconnect_all_slots();
    SignalFakeClientServerError.disconnect_all_slots();
  }

  bool FakeClientServer::Start(){
    if (!stop_){
      // LOG(ERROR) << "The FakeClientServer is not stop, you should not Start agian";
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
      boost::bind(&FakeClientServer::HandleStartAcceptor, shared_from_this()));
    return true;
  }

  void FakeClientServer::Stop(){
    if (stop_){
      return;
    }
    else{
      // Fast stop this server
      stop_ = true;
      accpet_service_.post(
        boost::bind(&FakeClientServer::HandleStopFakeClientServer, shared_from_this()));
    }
  }

  void FakeClientServer::HandleStartAcceptor(){
    if (stop_){
      // LOG(WARNING) << "The FakeClientServer was stoped";
      return;
    }
    if (!acceptor_){
      // LOG(ERROR) << "The Acceptor is null";
      return;
    }
    HttpConn::Ptr connect(HttpConn::CreateHttpConn(get_best_io_server()));
    acceptor_->async_accept(connect->socket(),
      boost::bind(&FakeClientServer::HandleAcceptor, shared_from_this(),
      connect, boost::asio::placeholders::error));
  }

  void FakeClientServer::HandleAcceptor(HttpConn::Ptr connect,
    const boost::system::error_code& err){

    if (stop_){
      // LOG(WARNING) << "The FakeClientServer was stoped";
      return;
    }

    if (err){
      stop_ = true;
      // LOG(ERROR) << err.message();
      return;
    }
    //SignalNewTcpConnect(shared_from_this(), connect);
    connect->StartHttpConn(
      boost::bind(&FakeClientServer::HandleHttpMethodComplete, shared_from_this(), _1, _2));
    HandleStartAcceptor();
  }

  void FakeClientServer::HandleStopFakeClientServer(){
    if (acceptor_){
      acceptor_->close();
    }
  }

  void FakeClientServer::HandleHttpMethodComplete(HttpConn::Ptr conn, Request& req){
    // DLOG(INFO) << req.method << " : " << req.host << ":" << req.port
   //    << " " << req.uri;
    FakeClientConnSession::Ptr conn_pair(new FakeClientConnSession(conn, 
      fake_server_, fake_port_));
    conn_pair->Start();
  }

  boost::asio::io_service& FakeClientServer::get_best_io_server(){
    io_services_.push_back(io_services_.front());
    io_services_.pop_front();
    return *io_services_.front();
  }
} // namespace proxy