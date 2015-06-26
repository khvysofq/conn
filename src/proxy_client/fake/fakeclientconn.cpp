#include "proxy_client/fake/fakeclientconn.h"

namespace proxy{


  FakeClientConn::FakeClientConn(boost::asio::io_service& io_service)
  :io_service_(io_service),
  socket_(io_service){
    key_ = rand() % MAX_KEY_RAND_SIZE + RAND_START_CHAR;
  }

  FakeClientConn::~FakeClientConn(){
    // DLOG(INFO) << "Delete fake client conn";
  }

  void FakeClientConn::StartConnectRequest(ConnectRequestCallback call_back,
    const std::string host, const std::string port){
    SignalConnectRequest = call_back;
    BuildConnectRequest(host, port);
    boost::asio::async_write(socket_,
      boost::asio::buffer(request_buffer_.Data(), request_buffer_.Length()),
      boost::bind(&FakeClientConn::HandleConnectRequestWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }
  void FakeClientConn::BuildConnectRequest(const std::string host, 
    const std::string port){
    unsigned char key = key_;
    char temp[256];
    std::string host_port = host + ":" + port;
    // DLOG(INFO) << host_port << "Key = " << (unsigned int)key_;
    request_buffer_.WriteString(FAKE_HEADER_REQUEST_START);
    request_buffer_.WriteUInt8(SPACE[0]);
    request_buffer_.WriteUInt8(SLASH);
    request_buffer_.WriteUInt8(CHA_KEY);
    request_buffer_.WriteUInt8(key_);
    request_buffer_.WriteUInt8(CHA_TYPE);
    request_buffer_.WriteUInt8(CHAR_TYPE_CONNECT);
    request_buffer_.WriteString(FAKE_HEADER_REQUES_END);
    request_buffer_.WriteUInt8(SPACE[0]);
    request_buffer_.WriteString(FAKE_HEADER_REQUEST_END);
    request_buffer_.WriteString(FAKE_CLIENT_HEADER);
    request_buffer_.WriteString(FAKE_CONTENT_LENGTH);
    // Write the content length
    sprintf(temp, "%u", (unsigned int)host_port.length());
    request_buffer_.WriteString(temp);
    request_buffer_.WriteUInt8(CRLF[0]);
    request_buffer_.WriteUInt8(CRLF[1]);
    request_buffer_.WriteUInt8(CRLF[0]);
    request_buffer_.WriteUInt8(CRLF[1]);
    const char* p = host_port.c_str();
    for (std::size_t i = 0; i < host_port.length(); i++){
      key = p[i] ^ key;
      temp[i] = key;
    }
    request_buffer_.WriteBytes(temp, host_port.size());
  }

  void FakeClientConn::HandleConnectRequestWrite(
    const boost::system::error_code& err){
    if (err){
      // DLOG(ERROR) << err.message();
      SignalConnectRequest(shared_from_this(), false);
      return;
    }
    request_buffer_.Clear();
    // Waiting for connect request result
    socket_.async_read_some(
      boost::asio::buffer(read_buffer_, MAX_READ_BUFFER),
      boost::bind(&FakeClientConn::HandleConnectRequestRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void FakeClientConn::HandleConnectRequestRead(
    const boost::system::error_code& err, int size){
    if (err){
      // DLOG(ERROR) << err.message();
      SignalConnectRequest(shared_from_this(), false);
      return;
    }
    // Parser the result
    request_buffer_.WriteBytes(read_buffer_, size);
    boost::tribool valid_request;
    int parser_size = 0;
    boost::tie(valid_request, parser_size) =
      http_parser_.ParserFakeResult(fake_res, 
      request_buffer_.Data(), request_buffer_.Length(), key_);
    if (boost::indeterminate(valid_request)){
      // Need more data
      socket_.async_read_some(
        boost::asio::buffer(read_buffer_, MAX_READ_BUFFER),
        boost::bind(&FakeClientConn::HandleConnectRequestRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else if (valid_request){
      if (fake_res.state_code == STATE_CONNECT_SUCCEED){
        SignalConnectRequest(shared_from_this(), true);
      }
      else{
        SignalConnectRequest(shared_from_this(), false);
      }
    }
    else{
      SignalConnectRequest(shared_from_this(), false);
    }
  }
}