#ifndef PROXY_CLIENT_FAKE_SERVER_CONN_H_
#define PROXY_CLIENT_FAKE_SERVER_CONN_H_

#include "proxy_client/fake/fakedefine.h"
#include "proxy_client/http/http_parser.h"

namespace proxy{

  class FakeServerConn : public boost::noncopyable,
    public boost::enable_shared_from_this<FakeServerConn>{
  public:
    typedef boost::shared_ptr<FakeServerConn> Ptr;

    FakeServerConn(boost::asio::io_service& io_service);
    virtual ~FakeServerConn();

    // public method
    boost::asio::ip::tcp::socket& Socket(){ return socket_; }
    boost::asio::io_service& io_service(){ return io_service_; }

    // Start Read the request
    void Start();
  private:
    void HandleReadConnectRequest(const boost::system::error_code& err, int size);

    void StartConnectServer(const std::string host, const std::string port);
    void HandleResolve(const boost::system::error_code& err,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void ConnectServer(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void HandleConnectServer(const boost::system::error_code& err,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    // Reply with 400 Not found
    void ReplyWith400Bad();
    void HandleReplyWrite(boost::system::error_code err);

    void OnServerConnectSucceed();
    void OnServerRead(boost::system::error_code err, int size);
    void OnServerWrite(boost::system::error_code err);
    void OnBrowerRead(boost::system::error_code err, int size);
    void OnBrowerWrite(boost::system::error_code err);
  private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::socket server_socket_;
    boost::asio::io_service& io_service_;

    static const int MAX_READ_BUFFER = 8192;
    char read_buffer_[MAX_READ_BUFFER];


    std::string host_;
    std::string port_;
    boost::asio::ip::tcp::resolver resolver_socket_;
    boost::scoped_ptr<boost::asio::ip::tcp::resolver::query> query_;
    boost::asio::ip::tcp::resolver::iterator end_;

    static const int MAX_PROXY_PACKET_SIZE = 1024 * 64;
    char brower_buffer_[MAX_PROXY_PACKET_SIZE];
    char server_buffer_[MAX_PROXY_PACKET_SIZE];

    // the key 
    char key_;
    vzconn::ByteBuffer request_buffer_;
    vzconn::ByteBuffer result_buffer_;
    HttpParser http_parser_;
    FakeRequest fake_res;
  };

}

#endif // PROXY_CLIENT_FAKE_SERVER_CONN_H_