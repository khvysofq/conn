#ifndef PROXY_CLIENT_FAKE_CLIENT_H_
#define PROXY_CLIENT_FAKE_CLIENT_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/fake/fakeclientconn.h"

namespace proxy{

  typedef boost::function < void(FakeClientConn::Ptr connect,
    const boost::system::error_code& err) > CallBackConnect;

  class FakeClient : public boost::noncopyable,
    public boost::enable_shared_from_this < FakeClient > {
  public:
    typedef boost::shared_ptr<FakeClient> Ptr;

    // -------------------------------------------------------------------------
    // Call once at there, this interface can't ganerator a FakeClient object
    static FakeClientConn::Ptr ConnectServer(boost::asio::io_service& io_service,
      const std::string addr, uint16 port, CallBackConnect call_back);
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    virtual ~FakeClient();

    static FakeClient::Ptr CreateFakeClient(boost::asio::io_service& io_service,
      const std::string addr, uint16 port);
    // -------------------------------------------------------------------------
  private:
    FakeClient(boost::asio::io_service& io_service,
      const boost::asio::ip::tcp::endpoint& addr);

    static void HandleStaticServerConnect(FakeClientConn::Ptr connect,
      const boost::system::error_code& err, CallBackConnect call_back);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::endpoint server_addr_;
  };
}

#endif // PROXY_CLIENT_FAKE_CLIENT_H_