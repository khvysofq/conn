#ifndef PROXY_FAKE_CLIENT_CONN_SESSION_H_
#define PROXY_FAKE_CLIENT_CONN_SESSION_H_

#include "proxy_client/conn/httpconn.h"
#include "proxy_client/fake/fakeclientconn.h"

namespace proxy{

  class FakeClientConnSession : public boost::noncopyable,
    public boost::enable_shared_from_this<FakeClientConnSession>{
  public:
    typedef boost::shared_ptr<FakeClientConnSession> Ptr;
    FakeClientConnSession(HttpConn::Ptr http_conn,
      std::string fake_server, unsigned short fake_port);
    virtual ~FakeClientConnSession();
    void Start();
  private:
    void HandleResolve(const boost::system::error_code& err,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void ConnectServer(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void HandleConnectServer(FakeClientConn::Ptr connect,
      const boost::system::error_code& err);
    virtual void OnServerConnectSucceed();

    void OnConnectRequest(FakeClientConn::Ptr fake_conn, bool succeed);

    void OnServerRead(boost::system::error_code err, int size);
    void OnServerWrite(boost::system::error_code err);
    void OnBrowerRead(boost::system::error_code err, int size);
    void OnBrowerWrite(boost::system::error_code err);

    HttpConn::Ptr http_conn_;
    boost::asio::ip::tcp::socket& brower_socket_;
    boost::asio::ip::tcp::resolver resolver_socket_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::resolver::query query_;
    boost::asio::ip::tcp::resolver::iterator end_;

    static const int MAX_PROXY_PACKET_SIZE = 1024 * 64;
    char brower_buffer_[MAX_PROXY_PACKET_SIZE];
    char server_buffer_[MAX_PROXY_PACKET_SIZE];

    FakeClientConn::Ptr fake_conn_;
    vzconn::ByteBuffer read_buffer_;
    vzconn::ByteBuffer write_buffer_;

    static int pair_count_;
    std::string fake_server_;
    unsigned short fake_port_;
  };
}

#endif // PROXY_FAKE_CLIENT_CONN_SESSION_H_