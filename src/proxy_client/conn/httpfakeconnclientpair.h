#ifndef PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_PAIR_H_
#define PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_PAIR_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/conn/httpconn.h"

namespace proxy{

  class HttpFakeConnClientPair : public boost::noncopyable,
    public boost::enable_shared_from_this < HttpFakeConnClientPair > {
  public:
    typedef boost::shared_ptr<HttpFakeConnClientPair> Ptr;
    HttpFakeConnClientPair(HttpConn::Ptr http_conn);
    virtual ~HttpFakeConnClientPair();
    void Start();

  private:

    HttpConn::Ptr http_conn_;
    boost::asio::ip::tcp::socket& brower_socket_;
    boost::asio::ip::tcp::socket server_socket_;
    boost::asio::ip::tcp::resolver resolver_socket_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::resolver::query query_;
    boost::asio::ip::tcp::resolver::iterator end_;

    static const int MAX_PROXY_PACKET_SIZE = 1024 * 64;
    char brower_buffer_[MAX_PROXY_PACKET_SIZE];
    char server_buffer_[MAX_PROXY_PACKET_SIZE];

    static int pair_count_;
  };

} // namespace proxy

#endif // PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_PAIR_H_