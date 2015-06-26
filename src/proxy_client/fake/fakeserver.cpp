#include "proxy_client/fake/fakeserver.h"

namespace proxy{

  FakeServer::Ptr FakeServer::CreateFakeServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const std::string addr, uint16 port){

    // LOG(INFO) << addr << ":" << port;

    boost::asio::ip::tcp::endpoint bind_addr(
      boost::asio::ip::address().from_string(addr), port);

    return FakeServer::Ptr(new FakeServer(io_services, accpet_service, bind_addr));
  }

  FakeServer::FakeServer(ios_deque& io_services,
    boost::asio::io_service& accpet_service,
    const boost::asio::ip::tcp::endpoint& bind_addr)
    :io_services_(io_services),
    accpet_service_(accpet_service),
    bind_addr_(bind_addr),
    stop_(true){

  }

  FakeServer::~FakeServer(){
    SignalNewTcpConnect.disconnect_all_slots();
    SignalFakeServerError.disconnect_all_slots();
  }

  bool FakeServer::Start(){
    if (!stop_){
      // LOG(ERROR) << "The FakeServer is not stop, you should not Start agian";
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
      boost::bind(&FakeServer::HandleStartAcceptor, shared_from_this()));
    return true;
  }

  void FakeServer::Stop(){
    if (stop_){
      return;
    }
    else{
      // Fast stop this server
      stop_ = true;
      accpet_service_.post(
        boost::bind(&FakeServer::HandleStopFakeServer, shared_from_this()));
    }
  }

  void FakeServer::HandleStartAcceptor(){
    if (stop_){
      // LOG(WARNING) << "The FakeServer was stoped";
      return;
    }
    if (!acceptor_){
      // LOG(ERROR) << "The Acceptor is null";
      return;
    }
    FakeServerConn::Ptr connect(new FakeServerConn(get_best_io_server()));
    acceptor_->async_accept(connect->Socket(),
      boost::bind(&FakeServer::HandleAcceptor, shared_from_this(),
      connect, boost::asio::placeholders::error));
  }

  void FakeServer::HandleAcceptor(FakeServerConn::Ptr connect,
    const boost::system::error_code& err){

    if (stop_){
      // LOG(WARNING) << "The FakeServer was stoped";
      return;
    }

    if (err){
      stop_ = true;
      // LOG(ERROR) << err.message();
      return;
    }
    std::cout << "new connect" << std::endl;
    connect->Start();
    HandleStartAcceptor();
  }

  void FakeServer::HandleStopFakeServer(){
    if (acceptor_){
      acceptor_->close();
    }
  }

  boost::asio::io_service& FakeServer::get_best_io_server(){
    io_services_.push_back(io_services_.front());
    io_services_.pop_front();
    return *io_services_.front();
  }
}