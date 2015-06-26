#include "database_server/networkmanager.h"
#include "database_server/database_server_defines.h"

namespace database{

  ProxyServerManager::ProxyServerManager(boost::asio::io_service& io_service)
    :io_service_(io_service){

  }

  ProxyServerManager::~ProxyServerManager(){

  }

  bool ProxyServerManager::StartProxyServerManager(
    const std::string addr, uint16 port){
    if (!tcp_server_){
      LOG_ERROR << "Tcp server exsis" << std::endl;
      return false;
    }

    tcp_server_ = vzconn::TcpServer::CreateTcpServer(io_service_, addr, port);

    tcp_server_->SignalNewTcpConnect.connect(
      boost::bind(&ProxyServerManager::OnNewTcpConnect, this, _1, _2));
    tcp_server_->SignalTcpServerError.connect(
      boost::bind(&ProxyServerManager::OnTcpServerError, this, _1, _2));

    return tcp_server_->Start();
  }

  void ProxyServerManager::OnNewTcpConnect(vzconn::TcpServer::Ptr tcp_server,
    vzconn::TcpConnect::Ptr tcp_connect){
    PingSession::Ptr ping_session(new PingSession(tcp_connect));
    ping_session->Start();
  }

  void ProxyServerManager::OnTcpServerError(vzconn::TcpServer::Ptr tcp_server,
    const boost::system::error_code& err){
    LOG_ERROR << err.message() << std::endl;
  }

  // ---------------------------------------------------------------------------
  PingSession::PingSession(vzconn::TcpConnect::Ptr connect)
    :connect_(connect),
    io_service_(connect->io_service()){

  }

  PingSession::~PingSession(){

  }


  void PingSession::Start(){
    connect_->SignalConnectError.connect(
      boost::bind(&PingSession::OnConnectError, shared_from_this(), _1, _2));
    connect_->SignalConnectWrite.connect(
      boost::bind(&PingSession::OnConnectWrite, shared_from_this(), _1));
    connect_->SignalConnectRead.connect(
      boost::bind(&PingSession::OnConnectRead, shared_from_this(), _1, _2, _3, _4));
  }

  void PingSession::OnConnectError(vzconn::TcpConnect::Ptr connect,
    const boost::system::error_code& err){

  }

  void PingSession::OnConnectWrite(vzconn::TcpConnect::Ptr connect){

  }

  void PingSession::OnConnectRead(vzconn::TcpConnect::Ptr connect,
    const char* buffer, int size, int flag){

  }

}