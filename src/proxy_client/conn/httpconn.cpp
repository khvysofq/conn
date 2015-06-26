#include "proxy_client/conn/httpconn.h"

namespace proxy{

  HttpConn::Ptr HttpConn::CreateHttpConn(
    boost::asio::io_service& io_service){
    return HttpConn::Ptr(new HttpConn(io_service));
  }

  HttpConn::HttpConn(boost::asio::io_service& io_service)
    :io_service_(io_service),
    socket_(io_service),
    state_(CS_CONNECTED){
  }

  HttpConn::~HttpConn(){
    //// DLOG(WARNING) << "Delete Httpconn";
  }

  void HttpConn::StartHttpConn(HttpMethoddCompleteCallback callback){
    SignalHttpMethodComplete = callback;

    socket_.async_read_some(
      boost::asio::buffer(buffer_, MAX_HTTP_BUFFER_SIZE),
      boost::bind(&HttpConn::OnHandleSocketRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void HttpConn::OnHandleSocketRead(const boost::system::error_code& err, int read_size){
    if (err){
      //// DLOG(ERROR) << err.message() << " " << bytebuffer_.Length();
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
        boost::bind(&HttpConn::OnHandleSocketRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else if(valid_request_){
      SignalHttpMethodComplete(shared_from_this(), req_);
      bytebuffer_.Consume(parser_size);
    }
    else{
      // DLOG(ERROR) << "Parser the http request error";
      // DLOG(ERROR).write(bytebuffer_.Data(), bytebuffer_.Length());
      Respone400Error();
    }
  }

  void HttpConn::Respone400Error(){
    reply_ = Reply::stock_reply(Reply::bad_request);
    AsyncReply(reply_);
  }

  void HttpConn::Respone404Error(){
    reply_ = Reply::stock_reply(Reply::not_found);
    AsyncReply(reply_);
  }

  void HttpConn::Respone503Error(){
    reply_ = Reply::stock_reply(Reply::service_unavailable);
    AsyncReply(reply_);
  }

  void HttpConn::AsyncReply(Reply& reply){
    boost::asio::async_write(socket_, reply.to_buffers(),
      boost::bind(&HttpConn::HandleAsyncWrite, shared_from_this(),
      boost::asio::placeholders::error));
  }

  void HttpConn::HandleAsyncWrite(boost::system::error_code err){
    // Initiate graceful connection closure.
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
    //// DLOG(INFO) << "Socket shutdown_both";
  }
}