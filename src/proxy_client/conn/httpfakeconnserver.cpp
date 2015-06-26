#include "proxy_client/conn/httpfakeconnserver.h"

namespace proxy{

  HttpFakeServerConn::HttpFakeServerConn(boost::asio::io_service& io_service)
    :io_service_(io_service),
    brower_socket_(io_service_){

  }

  HttpFakeServerConn::~HttpFakeServerConn(){

  }

  void HttpFakeServerConn::Start(){
    brower_socket_.async_read_some(
      boost::asio::buffer(brower_read_buffer_, MAX_SOCKET_BUFFER),
      boost::bind(&HttpFakeServerConn::HandleConnectRequestRead, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void HttpFakeServerConn::HandleConnectRequestRead(
    const boost::system::error_code& err, int size){
    if (err){
      // Read data error, not need reply
      DLOG(ERROR) << err.message();
      return;
    }

    // Save the data to the buffer
    brower_socket_buffer_.WriteBytes(brower_read_buffer_, size);
    boost::tribool valid_request;

  }
}