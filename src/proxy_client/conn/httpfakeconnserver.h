#ifndef PROXY_CLIENT_HTTP_FAKE_CONN_SERVER_H_
#define PROXY_CLIENT_HTTP_FAKE_CONN_SERVER_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/http/http_parser.h"

namespace proxy{
  // 1. Read the Request
  // 2. Connect the destiny server
  // 3. Send Result
  // 4. Exchange data
  class HttpFakeServerConn : public boost::noncopyable,
    public boost::enable_shared_from_this<HttpFakeServerConn>{
  public:
    HttpFakeServerConn(boost::asio::io_service& io_service);
    virtual ~HttpFakeServerConn();

    // Start the HttpFakeServerConn
    // 1. Read the request data
    void Start();
  private:
    void HandleConnectRequestRead(const boost::system::error_code& err, int size);
  private:
    static const int MAX_SOCKET_BUFFER = 8096;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket brower_socket_;
    vzconn::ByteBuffer brower_socket_buffer_;
    char brower_read_buffer_[MAX_SOCKET_BUFFER];
    HttpParser http_parser_;
  };

}

#endif // PROXY_CLIENT_HTTP_FAKE_CONN_SERVER_H_