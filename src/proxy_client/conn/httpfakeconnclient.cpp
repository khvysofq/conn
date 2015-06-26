#include "proxy_client/conn/httpfakeconnclient.h"
#include "proxy_client/http/header.h"

namespace proxy{

  std::string FakeRequest::fake_head = 
    "POST /fd/ls/lsp.aspx HTTP/1.1\r\n"
    "Accept: */*\r\n"
    "User-Agent: User-Agent	Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n"
    "Host:	www.bing.com\r\n"
    "Content-Type: text/xml\r\n";

  const std::string CONTENT_LENGTH = "Content-Length: ";
  const std::string COOKIE = "cookie: key=r";

  std::vector<boost::asio::const_buffer>& FakeRequest::to_buffers(
    std::vector<boost::asio::const_buffer>& buffers){
    buffers.push_back(boost::asio::buffer(fake_head));
    buffers.push_back(boost::asio::buffer(content_lenth.name));
    buffers.push_back(boost::asio::buffer(content_lenth.value));
    buffers.push_back(boost::asio::buffer(CRCF));
    buffers.push_back(boost::asio::buffer(cookie.name));
    buffers.push_back(boost::asio::buffer(cookie.value));
    buffers.push_back(boost::asio::buffer(CRCF));
  }

  HttpFakeConnClient::Ptr HttpFakeConnClient::CreateHttpFakeConnClient(
    boost::asio::io_service& io_service){
    return HttpFakeConnClient::Ptr(new HttpFakeConnClient(io_service));
  }

  HttpFakeConnClient::HttpFakeConnClient(boost::asio::io_service& io_service)
    :io_service_(io_service),
    socket_(io_service),
    state_(CS_CONNECTED){
    fake_req_.content_lenth.name = CONTENT_LENGTH;
    fake_req_.cookie.name = COOKIE;
    fake_req_.cookie.value = "TYPE=";
    key_ = rand() % 128;
  }

  HttpFakeConnClient::~HttpFakeConnClient(){
    //DLOG(WARNING) << "Delete Httpconn";
  }

  bool HttpFakeConnClient::StartHttpConnectRequest(HttpConnectCompleteCallback call_back,
    const std::string host, int port){
    fake_req_.content_lenth.value = "0";
    fake_req_.cookie.value.push_back(key_);
    fake_req_.to_buffers(write_buffer_);
    boost::asio::async_write(socket_,
      write_buffer_,
      boost::bind(&HttpFakeConnClient::HandleConnectRequest, shared_from_this(),
      boost::asio::placeholders::error, call_back));
  }

  void HttpFakeConnClient::HandleConnectRequest(const boost::system::error_code& err){
    if (err){
      DLOG(ERROR) << err.message();
      SignalHttpConnectComplete(shared_from_this(), false);
      return;
    }
    // Wait for connect request result
    socket_.async_read_some(
      boost::asio::buffer(buffer_, MAX_HTTP_BUFFER_SIZE),
      boost::bind(&HttpFakeConnClient::HandleConnectRequestResult, 
      shared_from_this(), boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void HttpFakeConnClient::HandleConnectRequestResult(
    const boost::system::error_code& err, int size){
    if (err){
      DLOG(ERROR) << err.message();
      SignalHttpConnectComplete(shared_from_this(), false);
      return;
    }
    // Parser the request result
    request_result_buffer_.WriteBytes(buffer_, size);
  }

  void HttpFakeConnClient::OnHandleSocketRead(
    const boost::system::error_code& err, int read_size){
    if (err){
      //DLOG(ERROR) << err.message() << " " << bytebuffer_.Length();
      return;
    }
    bytebuffer_.WriteBytes(buffer_, read_size);
    int parser_size = 0;
    boost::tie(valid_request_, parser_size) =
      http_parser_.parse(req_, bytebuffer_.Data(), bytebuffer_.Length());
    if (boost::indeterminate(valid_request_)){
      // Need more data
      socket_.async_read_some(
        boost::asio::buffer(buffer_, MAX_HTTP_BUFFER_SIZE),
        boost::bind(&HttpFakeConnClient::OnHandleSocketRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else if (valid_request_){
      SignalHttpMethodComplete(shared_from_this(), req_);
      bytebuffer_.Consume(parser_size);
    }
    else{
      DLOG(ERROR) << "Parser the http request error";
      DLOG(ERROR).write(bytebuffer_.Data(), bytebuffer_.Length());
    }
  }

  void HttpFakeConnClient::HandleAsyncWrite(boost::system::error_code err){
    // Initiate graceful connection closure.
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
    //DLOG(INFO) << "Socket shutdown_both";
  }

  // ---------------------------------------------------------------------------
  TcpClient::Ptr TcpClient::CreateTcpClient(boost::asio::io_service& io_service,
    const std::string addr, uint16 port){

    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    return TcpClient::Ptr(new TcpClient(io_service, server_addr));
  }

  HttpFakeConnClient::Ptr TcpClient::ConnectServer(boost::asio::io_service& io_service,
    const std::string addr, uint16 port, CallBackConnect call_back){

    HttpFakeConnClient::Ptr connect(HttpFakeConnClient::CreateHttpFakeConnClient(io_service));

    boost::asio::ip::tcp::endpoint server_addr(
      boost::asio::ip::address().from_string(addr), port);

    connect->socket().async_connect(server_addr,
      boost::bind(&TcpClient::HandleStaticServerConnect, connect,
      boost::asio::placeholders::error, call_back));
    return connect;
  }

  void TcpClient::HandleStaticServerConnect(HttpFakeConnClient::Ptr connect,
    const boost::system::error_code& err, CallBackConnect call_back){
      call_back(connect, err);
  }

  TcpClient::TcpClient(boost::asio::io_service& io_service,
    const boost::asio::ip::tcp::endpoint& addr)
    :io_service_(io_service),
    server_addr_(addr){

  }

  TcpClient::~TcpClient(){
  }

}