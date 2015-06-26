#ifndef LIBVZCONN_TCP_CLIENT_H_
#define LIBVZCONN_TCP_CLIENT_H_

#include "vzconn/conn/tcpconnect.h"

namespace vzconn{

  typedef boost::function < void(TcpConnect::Ptr connect,
    const boost::system::error_code& err) > CallBackConnect;

  class TcpClient : public boost::noncopyable,
    public boost::enable_shared_from_this < TcpClient > {
  public:
    typedef boost::shared_ptr<TcpClient> Ptr;

    // -------------------------------------------------------------------------
    // Call once at there, this interface can't ganerator a TcpClient object
    static TcpConnect::Ptr ConnectServer(boost::asio::io_service& io_service,
      const std::string addr, uint16 port, CallBackConnect call_back);
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    virtual ~TcpClient();

    static TcpClient::Ptr CreateTcpClient(boost::asio::io_service& io_service,
      const std::string addr, uint16 port);

    boost::signals2::signal <void(TcpClient::Ptr tcp_client, 
      TcpConnect::Ptr connect, const boost::system::error_code& err)> SignalConnect;

    TcpConnect::Ptr Connect();
    // -------------------------------------------------------------------------
  private:
    TcpClient(boost::asio::io_service& io_service,
      const boost::asio::ip::tcp::endpoint& addr);

    void HandleServerConnect(TcpConnect::Ptr connect,
      const boost::system::error_code& err);
    static void HandleStaticServerConnect(TcpConnect::Ptr connect,
      const boost::system::error_code& err, CallBackConnect call_back);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::endpoint server_addr_;
  };

}

#endif // LIBVZCONN_TCP_CLIENT_H_