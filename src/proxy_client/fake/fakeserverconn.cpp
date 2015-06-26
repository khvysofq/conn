#include "proxy_client/fake/fakeserverconn.h"
#include "proxy_client/fake/fakeparser.h"


namespace proxy{

  FakeServerConn::FakeServerConn(boost::asio::io_service& io_service)
    :io_service_(io_service),
    socket_(io_service),
    resolver_socket_(io_service),
    server_socket_(io_service){

  }

  FakeServerConn::~FakeServerConn(){

  }

  void FakeServerConn::Start(){
    // Start read the connect request and the key
    socket_.async_read_some(
      boost::asio::buffer(read_buffer_, MAX_READ_BUFFER),
      boost::bind(&FakeServerConn::HandleReadConnectRequest, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void FakeServerConn::HandleReadConnectRequest(
    const boost::system::error_code& err, int size){
    if (err){
      // DLOG(ERROR) << err.message();
      return;
    }
    request_buffer_.WriteBytes(read_buffer_, size);
    boost::tribool valid_request;
    int parser_size = 0;
    boost::tie(valid_request, parser_size) =
      http_parser_.ParserFakeRequest(fake_res,
      request_buffer_.Data(), request_buffer_.Length(), key_);
    if (boost::indeterminate(valid_request)){
      // Need more data
      socket_.async_read_some(
        boost::asio::buffer(read_buffer_, MAX_READ_BUFFER),
        boost::bind(&FakeServerConn::HandleReadConnectRequest, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else if (valid_request){
      // get the host and port
      host_.clear();
      port_.clear();
      bool rh = false;
      const char *p = fake_res.content.c_str();
      for (std::size_t i = 0; i < fake_res.content.size(); i++){
        if (p[i] == ':'){
          rh = true;
        }
        else if (rh){
          port_.push_back(p[i]);
        }
        else{
          host_.push_back(p[i]);
        }
      }
      if (host_.empty() || port_.empty()){
        // false to read the host and port
        ReplyWith400Bad();
      }
      else{
        // Start connect the server
        StartConnectServer(host_, port_);
      }
    }
    else{
      ReplyWith400Bad();
    }
  }

  void FakeServerConn::StartConnectServer(const std::string host, const std::string port){
    std::cout << host << " : " << port << "key = " << (unsigned int)key_ << std::endl;
    query_.reset(new boost::asio::ip::tcp::resolver::query(host, port));

    resolver_socket_.async_resolve(*query_,
      boost::bind(&FakeServerConn::HandleResolve, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::iterator));
  }

  void FakeServerConn::HandleResolve(const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    if (err){
      // LOG(ERROR) << err.message();
      ReplyWith400Bad();
      return;
    }
    ConnectServer(endpoint_iterator);
  }

  void FakeServerConn::ConnectServer(
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    boost::asio::ip::tcp::endpoint server_addr = *endpoint_iterator;
    server_socket_.async_connect(server_addr,
      boost::bind(&FakeServerConn::HandleConnectServer, shared_from_this(),
      boost::asio::placeholders::error, ++endpoint_iterator));
  }

  void FakeServerConn::HandleConnectServer(const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator){
    if (err && endpoint_iterator == end_){
      // Sure connect failure
      // LOG(ERROR) << err.message() << ":" << host_;
      ReplyWith400Bad();
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

  void FakeServerConn::ReplyWith400Bad(){
    boost::asio::async_write(socket_,
      boost::asio::buffer(FAKE_RESULT_400),
      boost::bind(&FakeServerConn::HandleReplyWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void FakeServerConn::HandleReplyWrite(boost::system::error_code err){
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
    // Do nothing
  }

  void FakeServerConn::OnServerConnectSucceed(){
    // Read the brower socket
    // 
    // Reply with Connect succeed
    // BR <-> SW
    boost::asio::async_write(socket_,
      boost::asio::buffer(FAKE_RESULT_200),
      boost::bind(&FakeServerConn::OnServerWrite, shared_from_this(),
      boost::asio::placeholders::error));
    // SR <-> BW
    server_socket_.async_read_some(
      boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&FakeServerConn::OnServerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void FakeServerConn::OnServerRead(boost::system::error_code err, int size){
    if (err){
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << host_ << " : " << port_;
      return;
    }
    ConstCharToBytebufferD(server_buffer_, size, &result_buffer_, key_);
    // Write to the brower socket
    boost::asio::async_write(socket_,
      boost::asio::buffer(result_buffer_.Data(), result_buffer_.Length()),
      boost::bind(&FakeServerConn::OnBrowerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void FakeServerConn::OnServerWrite(boost::system::error_code err){
    if (err){
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << host_ << " : " << port_;
      return;
    }
    request_buffer_.Consume(request_buffer_.Length());
    // Read brower socket read
    socket_.async_read_some(
      boost::asio::buffer(brower_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&FakeServerConn::OnBrowerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void FakeServerConn::OnBrowerRead(boost::system::error_code err, int size){
    if (err){
      server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << host_ << " : " << port_;
      return;
    }
    ConstCharToBytebufferE(brower_buffer_, size, &request_buffer_, key_);
    // Write data to the server socket
    boost::asio::async_write(server_socket_,
      boost::asio::buffer(request_buffer_.Data(), request_buffer_.Length()),
      boost::bind(&FakeServerConn::OnServerWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void FakeServerConn::OnBrowerWrite(boost::system::error_code err){
    if (err){
      server_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
      //// LOG(ERROR) << err.message() << ":" << host_ << " : " << port_;
      return;
    }
    result_buffer_.Consume(result_buffer_.Length());
    // Read server socket data
    server_socket_.async_read_some(
      boost::asio::buffer(server_buffer_, MAX_PROXY_PACKET_SIZE),
      boost::bind(&FakeServerConn::OnServerRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }
}