#ifndef PROXY_FAKE_CLIENT_CONN_H_
#define PROXY_FAKE_CLIENT_CONN_H_

#include "proxy_client/fake/fakedefine.h"
#include "proxy_client/http/http_parser.h"

namespace proxy{

  class FakeClientConn : public boost::noncopyable,
    public boost::enable_shared_from_this < FakeClientConn > {
  public:
    typedef boost::shared_ptr<FakeClientConn> Ptr;
    typedef boost::function < void(FakeClientConn::Ptr fake_conn,
      bool succeed) > ConnectRequestCallback;

    FakeClientConn(boost::asio::io_service& io_service);
    virtual ~FakeClientConn();

    // public method
    boost::asio::ip::tcp::socket& Socket(){ return socket_; }
    boost::asio::io_service& io_service(){ return io_service_; }
    char Key(){ return key_; }

    void StartConnectRequest(ConnectRequestCallback call_back, 
      const std::string host, const std::string port);
  private:
    // Help callback method
    void HandleConnectRequestWrite(const boost::system::error_code& err);
    void HandleConnectRequestRead(const boost::system::error_code& err, int size);
  private:
    // Http header fake method
    void BuildConnectRequest(const std::string host, const std::string port);
  private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_service& io_service_;
    
    // const
    static const int MAX_READ_BUFFER = 1024 * 64;
    char read_buffer_[MAX_READ_BUFFER];

    // Connect request member
    ConnectRequestCallback SignalConnectRequest;

    // the key 
    char key_;
    vzconn::ByteBuffer request_buffer_;
    HttpParser http_parser_;
    FakeResult fake_res;
  };


};

#endif // PROXY_FAKE_CLIENT_CONN_H_