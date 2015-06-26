#include "proxy_client/conn/httpconnpair.h"

namespace proxy{

  static const std::string connect_established = "HTTP/1.1 200 Connection Established\r\n"
    "Proxy-agent: Netscape-Proxy/1.1\r\n\r\n";
  int HttpConnPair::pair_count_ = 0;

  HttpConnPair::HttpConnPair(HttpConn::Ptr http_conn)
    :http_conn_(http_conn),
    brower_socket_(http_conn->socket()),
    io_service_(http_conn->io_service()),
    server_socket_(http_conn->io_service()),
    resolver_socket_(http_conn->io_service()),
    query_(http_conn->get_reqeust().host, http_conn->get_reqeust().port){
    pair_count_++;
    // // DLOG(WARNING) << "New http conn pair " << pair_count_;
  }

  HttpConnPair::~HttpConnPair(){
    pair_count_--;
    // // DLOG(WARNING) << "Delete http conn pair " << pair_count_;
  }

  void HttpConnPair::Start(){
    resolver_socket_.async_resolve(query_,
      boost::bind(&HttpConnPair::HandleResolve, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::iterator));
  }

  void HttpConnPair::HandleResolve(const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    if (err){
      // LOG(ERROR) << err.message();
      http_conn_->Respone503Error();
      return;
    }
    ConnectServer(endpoint_iterator);
  }

  void HttpConnPair::ConnectServer(
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    boost::asio::ip::tcp::endpoint server_addr = *endpoint_iterator;
    server_socket_.async_connect(server_addr,
      boost::bind(&HttpConnPair::HandleConnectServer, shared_from_this(),
      boost::asio::placeholders::error, ++endpoint_iterator));
  }

  void HttpConnPair::HandleConnectServer(const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    if (err && endpoint_iterator == end_){
      // Sure connect failure
      // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      http_conn_->Respone503Error();
      return;
    }
    if (err){
      //// LOG(WARNING) << "Next Endpoint " << err.message() << ":" 
      //  << http_conn_->get_reqeust().host;
      // Connect to the next endpoint
      ConnectServer(endpoint_iterator);
    }
    else{
      OnServerConnectSucceed();
    }
  }

  void HttpConnPair::OnServerConnectSucceed(){
    if (http_conn_->get_reqeust().method == HTTP_CONNECT){
      // Reply with Connect succeed
      // BW <-> SR
      boost::asio::async_write(brower_socket_,
        boost::asio::buffer(connect_established, connect_established.length()),
        boost::bind(&HttpConnPair::OnBrowerWrite, shared_from_this(),
        boost::asio::placeholders::error));
      // BR <-> SW
      brower_socket_.async_read_some(
        boost::asio::buffer(brower_buffer_, MAX_PROXY_PACKET_SIZE),
        boost::bind(&HttpConnPair::OnBrowerRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else{
      // Reply with Connect succeed
      // SW <-> BR
      boost::asio::async_write(server_socket_,
        http_conn_->get_reqeust().to_proxy_buffers(),
        boost::bind(&HttpConnPair::OnServerWrite, shared_from_this(),
        boost::asio::placeholders::error));
      // SR <-> BW
      server_socket_.async_read_some(
        boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
        boost::bind(&HttpConnPair::OnServerRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
  }

  void HttpConnPair::OnServerRead(boost::system::error_code err, int size){
    if (err){
      brower_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      // // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    // Write to the brower socket
    boost::asio::async_write(brower_socket_,
      boost::asio::buffer(server_buffer_, size),
      boost::bind(&HttpConnPair::OnBrowerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void HttpConnPair::OnServerWrite(boost::system::error_code err){
    if (err){
      brower_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      // // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    // Read brower socket read
    brower_socket_.async_read_some(
      boost::asio::buffer(brower_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&HttpConnPair::OnBrowerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void HttpConnPair::OnBrowerRead(boost::system::error_code err, int size){
    if (err){
      server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      // // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    // Write data to the server socket
    boost::asio::async_write(server_socket_,
      boost::asio::buffer(brower_buffer_, size),
      boost::bind(&HttpConnPair::OnServerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void HttpConnPair::OnBrowerWrite(boost::system::error_code err){
    if (err){
      server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      // // LOG(ERROR) << err.message() << ":" << http_conn_->get_reqeust().host;
      return;
    }
    // Read server socket data
    server_socket_.async_read_some(
      boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&HttpConnPair::OnServerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }
}