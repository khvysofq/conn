#ifndef LIBVZCONN_CONN_TCPSERVER_H_
#define LIBVZCONN_CONN_TCPSERVER_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/conn/httpconn.h"
#include <queue>

namespace ba = boost::asio;
namespace bs = boost::system;

typedef boost::shared_ptr<ba::ip::tcp::socket> socket_ptr;
typedef boost::weak_ptr<ba::ip::tcp::socket> socket_wptr;
typedef boost::shared_ptr<ba::io_service> io_service_ptr;

typedef std::deque<io_service_ptr> ios_deque;

namespace proxy {


  class TcpServer : public boost::noncopyable,
    public boost::enable_shared_from_this<TcpServer>{
  public:
    typedef boost::shared_ptr<TcpServer> Ptr;

    static TcpServer::Ptr CreateTcpServer(ios_deque& io_services,
      boost::asio::io_service& accpet_service,
      const std::string addr, uint16 port);
    virtual ~TcpServer();

    // Signal a new tcp connected
    boost::signals2::signal < void(TcpServer::Ptr tcp_server,
      HttpConn::Ptr http_conn) >SignalNewTcpConnect;
    // Signal tcp server error
    boost::signals2::signal < void(HttpConn::Ptr http_conn,
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
    explicit TcpServer(ios_deque& io_services, 
      boost::asio::io_service& accpet_service,
      const boost::asio::ip::tcp::endpoint& bind_addr);

    void HandleStartAcceptor();
    void HandleAcceptor(HttpConn::Ptr new_connect,
      const boost::system::error_code& err);
    void HandleStopTcpServer();
    void HandleHttpMethodComplete(HttpConn::Ptr conn, Request& req);
  private:
    boost::asio::io_service& get_best_io_server();

    ios_deque io_services_;
    boost::asio::io_service& accpet_service_;
    boost::asio::ip::tcp::endpoint bind_addr_;
    boost::scoped_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    //boost::scoped_ptr<boost::asio::lo>
    bool stop_;
  };

}

#endif // LIBVZCONN_CONN_TCPSERVER_H_