#ifndef PROXY_CLIENT_CONN_HTTP_CONN_PAIR_H_
#define PROXY_CLIENT_CONN_HTTP_CONN_PAIR_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/conn/httpconn.h"

namespace proxy{
  
  class HttpConnPair : public boost::noncopyable, 
    public boost::enable_shared_from_this<HttpConnPair>{
  public:
    typedef boost::shared_ptr<HttpConnPair> Ptr;
    HttpConnPair(HttpConn::Ptr http_conn);
    virtual ~HttpConnPair();
    void Start();
  private:
    void HandleResolve(const boost::system::error_code& err,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void ConnectServer(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void HandleConnectServer(const boost::system::error_code& err,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    virtual void OnServerConnectSucceed();

    void OnServerRead(boost::system::error_code err, int size);
    void OnServerWrite(boost::system::error_code err);
    void OnBrowerRead(boost::system::error_code err, int size);
    void OnBrowerWrite(boost::system::error_code err);

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

}

#endif // PROXY_CLIENT_CONN_HTTP_CONN_PAIR_H_

