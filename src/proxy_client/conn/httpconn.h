#ifndef PROXY_CLIENT_HTTP_CONN_H_
#define PROXY_CLIENT_HTTP_CONN_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/http/http_parser.h"
#include "proxy_client/http/reply.h"

namespace proxy {

  class HttpConn : public boost::noncopyable,
    public boost::enable_shared_from_this<HttpConn>{
  public:
    enum ConnState{
      CS_CLOSED,
      CS_CONNECTED
    };
    typedef boost::shared_ptr<HttpConn> Ptr;
    typedef boost::function<void(HttpConn::Ptr conn, Request& req)> 
      HttpMethoddCompleteCallback;

    static HttpConn::Ptr CreateHttpConn(boost::asio::io_service& io_service);
    virtual ~HttpConn();
    boost::asio::ip::tcp::socket& socket(){ return socket_; }
    boost::asio::io_service& io_service(){ return io_service_; }
    Request& get_reqeust(){ return req_; }

    // Function method
    void StartHttpConn(HttpMethoddCompleteCallback callback);

    void Respone400Error();
    void Respone404Error();
    void Respone503Error();
  private:
    HttpConn(boost::asio::io_service& io_service);
    // callback method
    void OnHandleSocketRead(const boost::system::error_code& err, int read_size);

    void AsyncReply(Reply& reply);
    void HandleAsyncWrite(boost::system::error_code err);
  private:
    static const int MAX_HTTP_BUFFER_SIZE = 8 * 1024;
    ConnState state_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    char buffer_[MAX_HTTP_BUFFER_SIZE];

    Request req_;
    HttpParser http_parser_;
    boost::tribool valid_request_;

    vzconn::ByteBuffer bytebuffer_;

    Reply reply_;

    // Signal callback setting
    HttpMethoddCompleteCallback SignalHttpMethodComplete;
  };
}

#endif // PROXY_CLIENT_HTTP_CONN_H_