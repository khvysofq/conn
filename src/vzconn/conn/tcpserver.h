#ifndef LIBVZCONN_CONN_TCPSERVER_H_
#define LIBVZCONN_CONN_TCPSERVER_H_

#include "vzconn/base/basicdefines.h"
#include "vzconn/conn/tcpconnect.h"

namespace vzconn {

  class TcpServer : public boost::noncopyable,
    public boost::enable_shared_from_this<TcpServer>{
  public:
    typedef boost::shared_ptr<TcpServer> Ptr;

    static TcpServer::Ptr CreateTcpServer(boost::asio::io_service& io_service,
      const std::string addr, uint16 port);
    virtual ~TcpServer();

    // Signal a new tcp connected
    boost::signals2::signal < void(TcpServer::Ptr tcp_server, 
      TcpConnect::Ptr tcp_connect) >SignalNewTcpConnect;
    // Signal tcp server error
    boost::signals2::signal < void(TcpServer::Ptr tcp_server,
      const boost::system::error_code& err) > SignalTcpServerError;

    // Not Thread safe, only run at created io_service
    bool Start();

    // Thread safe
    void Stop();

    // Help function
    const boost::asio::ip::tcp::endpoint& bind_addr() const{
      return bind_addr_;
    }
  private:
    // Only alown one io thread
    explicit TcpServer(boost::asio::io_service& io_service,
      const boost::asio::ip::tcp::endpoint& bind_addr);

    void HandleStartAcceptor();
    void HandleAcceptor(TcpConnect::Ptr new_connect,
      const boost::system::error_code& err);
    void HandleStopTcpServer();
  private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::endpoint bind_addr_;
    boost::scoped_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    //boost::scoped_ptr<boost::asio::lo>
    bool stop_;
  };

}

#endif // LIBVZCONN_CONN_TCPSERVER_H_