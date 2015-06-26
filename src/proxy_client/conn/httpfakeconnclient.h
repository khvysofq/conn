#ifndef PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_H_
#define PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_H_

#include "proxy_client/base/proxybase.h"
#include "proxy_client/http/http_parser.h"
#include "proxy_client/http/reply.h"

namespace proxy{

  struct FakeRequest{
    static std::string fake_head;
    header content_lenth;
    header cookie;
    /// The optional content sent with the Request.
    std::string content;
    std::vector<boost::asio::const_buffer>& to_buffers(
      std::vector<boost::asio::const_buffer>& buffers);
  };

  class HttpFakeConnClient : public boost::noncopyable,
    public boost::enable_shared_from_this<HttpFakeConnClient>{
  public:
    enum ConnState{
      CS_CLOSED,
      CS_CONNECTED
    };
    typedef boost::shared_ptr<HttpFakeConnClient> Ptr;
    typedef boost::function<void(HttpFakeConnClient::Ptr conn, Request& req)>
      HttpMethoddCompleteCallback;
    typedef boost::function < void(HttpFakeConnClient::Ptr conn, bool succeed) >
      HttpConnectCompleteCallback;

    static HttpFakeConnClient::Ptr CreateHttpFakeConnClient(
      boost::asio::io_service& io_service);
    virtual ~HttpFakeConnClient();

    boost::asio::ip::tcp::socket& socket(){ return socket_; }
    boost::asio::io_service& io_service(){ return io_service_; }

    Request& get_reqeust(){ return req_; }

    bool StartHttpConnectRequest(HttpConnectCompleteCallback call_back,
      const std::string host, int port);

  private:
    HttpFakeConnClient(boost::asio::io_service& io_service);
    // callback method
    void OnHandleSocketRead(const boost::system::error_code& err, int read_size);

    void AsyncReply(Reply& reply);
    void HandleAsyncWrite(boost::system::error_code err);

    // Fake write
    void HandleConnectRequest(const boost::system::error_code& err);
    void HandleConnectRequestResult(const boost::system::error_code& err, int size);
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
    vzconn::ByteBuffer request_result_buffer_;
    std::vector<boost::asio::const_buffer> write_buffer_;
    HttpConnectCompleteCallback SignalHttpConnectComplete;

    Reply reply_;
    FakeRequest fake_req_;
    char key_;

    // Signal callback setting
    HttpMethoddCompleteCallback SignalHttpMethodComplete;
  };


  typedef boost::function < void(HttpFakeConnClient::Ptr connect,
    const boost::system::error_code& err) > CallBackConnect;

  class TcpClient : public boost::noncopyable,
    public boost::enable_shared_from_this < TcpClient > {
  public:
    typedef boost::shared_ptr<TcpClient> Ptr;

    // -------------------------------------------------------------------------
    // Call once at there, this interface can't ganerator a TcpClient object
    static HttpFakeConnClient::Ptr ConnectServer(boost::asio::io_service& io_service,
      const std::string addr, uint16 port, CallBackConnect call_back);
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    virtual ~TcpClient();

    static TcpClient::Ptr CreateTcpClient(boost::asio::io_service& io_service,
      const std::string addr, uint16 port);
    // -------------------------------------------------------------------------
  private:
    TcpClient(boost::asio::io_service& io_service,
      const boost::asio::ip::tcp::endpoint& addr);

    static void HandleStaticServerConnect(HttpFakeConnClient::Ptr connect,
      const boost::system::error_code& err, CallBackConnect call_back);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::endpoint server_addr_;
  };

}

#endif // PROXY_CLIENT_HTTP_FAKE_CONN_CLIENT_H_