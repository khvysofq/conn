#include "proxy_client/fake/fakeclientconnsession.h"
#include "proxy_client/fake/fakeclient.h"
#include "proxy_client/fake/fakeparser.h"

namespace proxy{

  static const std::string connect_established = "HTTP/1.1 200 Connection Established\r\n"
    "Proxy-agent: Netscape-Proxy/1.1\r\n\r\n";
  int FakeClientConnSession::pair_count_ = 0;

  FakeClientConnSession::FakeClientConnSession(HttpConn::Ptr http_conn,
    std::string fake_server, unsigned short fake_port)
    :http_conn_(http_conn),
    brower_socket_(http_conn->socket()),
    io_service_(http_conn->io_service()),
    resolver_socket_(http_conn->io_service()),
    query_(http_conn->get_reqeust().host, http_conn->get_reqeust().port),
    fake_server_(fake_server),
    fake_port_(fake_port){
    pair_count_++;
    // // DLOG(WARNING) << "New http conn pair " << pair_count_;
  }

  FakeClientConnSession::~FakeClientConnSession(){
    pair_count_--;
    // // DLOG(WARNING) << "Delete http conn pair " << pair_count_;
  }

  void FakeClientConnSession::Start(){
    fake_conn_ = FakeClient::ConnectServer(io_service_, fake_server_, fake_port_,
      boost::bind(&FakeClientConnSession::HandleConnectServer, shared_from_this(),
      _1,_2));
  }

  void FakeClientConnSession::HandleConnectServer(
    FakeClientConn::Ptr connect, const boost::system::error_code& err){
    BOOST_ASSERT(fake_conn_ == connect);
    if (err){
      // Sure connect failure
      // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      http_conn_->Respone503Error();
      return;
    }
    else{
      fake_conn_->StartConnectRequest(
        boost::bind(&FakeClientConnSession::OnConnectRequest, shared_from_this(),
        _1, _2), http_conn_->get_reqeust().host, http_conn_->get_reqeust().port);
    }
  }

  void FakeClientConnSession::OnConnectRequest(
    FakeClientConn::Ptr fake_conn, bool succeed){
    if (succeed){
      OnServerConnectSucceed();
    }
    else{
      // Sure connect failure
      // LOG(ERROR) << "Connect Request error " << fake_server_ << " : " << fake_port_;
      http_conn_->Respone503Error();
    }
  }

  void FakeClientConnSession::OnServerConnectSucceed(){
    if (http_conn_->get_reqeust().method == HTTP_CONNECT){
      // Reply with Connect succeed
      // BW <-> SR
      boost::asio::async_write(brower_socket_,
        boost::asio::buffer(connect_established, connect_established.length()),
        boost::bind(&FakeClientConnSession::OnBrowerWrite, shared_from_this(),
        boost::asio::placeholders::error));
      // BR <-> SW
      brower_socket_.async_read_some(
        boost::asio::buffer(brower_buffer_, MAX_PROXY_PACKET_SIZE),
        boost::bind(&FakeClientConnSession::OnBrowerRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else{
      // Reply with Connect succeed
      // SW <-> BR
      std::vector<boost::asio::const_buffer> const_buffer =
        http_conn_->get_reqeust().to_proxy_buffers();
      ConstBufferToBytebufferD(http_conn_->get_reqeust().to_proxy_buffers(),
        &write_buffer_, fake_conn_->Key());
      boost::asio::async_write(fake_conn_->Socket(),
        boost::asio::buffer(write_buffer_.Data(), write_buffer_.Length()),
        boost::bind(&FakeClientConnSession::OnServerWrite, shared_from_this(),
        boost::asio::placeholders::error));
      // SR <-> BW
      fake_conn_->Socket().async_read_some(
        boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
        boost::bind(&FakeClientConnSession::OnServerRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
  }

  void FakeClientConnSession::OnServerRead(boost::system::error_code err, int size){
    if (err){
      brower_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    ConstCharToBytebufferE(server_buffer_, size, &read_buffer_, fake_conn_->Key());
    //// DLOG(INFO).write(read_buffer_.Data(), read_buffer_.Length());
    // Write to the brower socket
    boost::asio::async_write(brower_socket_,
      boost::asio::buffer(read_buffer_.Data(), read_buffer_.Length()),
      boost::bind(&FakeClientConnSession::OnBrowerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void FakeClientConnSession::OnServerWrite(boost::system::error_code err){
    if (err){
      brower_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    write_buffer_.Consume(write_buffer_.Length());
    // Read brower socket read
    brower_socket_.async_read_some(
      boost::asio::buffer(brower_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&FakeClientConnSession::OnBrowerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void FakeClientConnSession::OnBrowerRead(boost::system::error_code err, int size){
    if (err){
      fake_conn_->Socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    ConstCharToBytebufferD(brower_buffer_, size, &write_buffer_, fake_conn_->Key());
    // Write data to the server socket
    boost::asio::async_write(fake_conn_->Socket(),
      boost::asio::buffer(write_buffer_.Data(), write_buffer_.Length()),
      boost::bind(&FakeClientConnSession::OnServerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void FakeClientConnSession::OnBrowerWrite(boost::system::error_code err){
    if (err){
      fake_conn_->Socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    read_buffer_.Consume(read_buffer_.Length());
    // Read server socket data
    fake_conn_->Socket().async_read_some(
      boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&FakeClientConnSession::OnServerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }
}