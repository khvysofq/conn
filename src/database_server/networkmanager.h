#ifndef DATABASE_SERVER_NETWORK_MANAGER_H_
#define DATABASE_SERVER_NETWORK_MANAGER_H_
#include "vzconn/conn/tcpserver.h"

namespace database{
  
  class PingSession;

  class ProxyServerManager : public boost::noncopyable{
  public:
    ProxyServerManager(boost::asio::io_service& io_service);
    virtual ~ProxyServerManager();

    bool StartProxyServerManager(const std::string addr, uint16 port);
  private:
    void OnNewTcpConnect(vzconn::TcpServer::Ptr tcp_server,
      vzconn::TcpConnect::Ptr tcp_connect);
    void OnTcpServerError(vzconn::TcpServer::Ptr tcp_server,
      const boost::system::error_code& err);
  private:
    boost::asio::io_service& io_service_;
    vzconn::TcpServer::Ptr tcp_server_;
  };

  class PingSession : public boost::noncopyable,
    public boost::enable_shared_from_this<PingSession>{
  public:
    typedef boost::shared_ptr<PingSession> Ptr;
    PingSession(vzconn::TcpConnect::Ptr connect);
    virtual ~PingSession();
    void Start();
  private:
    void OnConnectError(vzconn::TcpConnect::Ptr connect,
      const boost::system::error_code& err);
    void OnConnectWrite(vzconn::TcpConnect::Ptr connect);
    void OnConnectRead(vzconn::TcpConnect::Ptr connect,
      const char* buffer, int size, int flag);
  private:
    vzconn::TcpConnect::Ptr connect_;
    boost::asio::io_service& io_service_;
  };
}

#endif // DATABASE_SERVER_NETWORK_MANAGER_H_