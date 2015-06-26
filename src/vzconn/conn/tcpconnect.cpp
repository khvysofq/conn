#include "vzconn/conn/tcpconnect.h"

namespace vzconn{

  static const char* VZ_HEADER = "VZ";

  TcpConnect::Ptr TcpConnect::CreateTcpConnect(
    boost::asio::io_service& io_service){
    return TcpConnect::Ptr(new TcpConnect(io_service));
  }

  TcpConnect::TcpConnect(boost::asio::io_service& io_service)
    :io_service_(io_service),
    socket_(io_service),
    state_(CS_CONNECTED),
    atoms_buffer_(new ByteBuffer(ByteBuffer::ORDER_NETWORK)),
    write_buffer_(new ByteBuffer(ByteBuffer::ORDER_NETWORK)),
    read_buffer_(new ByteBuffer(ByteBuffer::ORDER_HOST)),
    is_writing_(false),
    temp_size_(0){
  }

  TcpConnect::~TcpConnect(){
    //DLOG(WARNING) << "Delete TcpConnect";
    if (state_ != CS_CLOSED && socket_.is_open()){
      socket_.close();
    }
    SignalConnectError.disconnect_all_slots();
    SignalConnectWrite.disconnect_all_slots();
    SignalConnectRead.disconnect_all_slots();
  }

  bool TcpConnect::AsyncWrite(const char* buffer, int size, int flag){
    
    if (state_ != CS_CONNECTED){
      //LOG(ERROR) << "The socket is not opened";
      return false;
    }

    if (size > MAX_PKT_SIZE){
      return false;
    }
    
    boost::mutex::scoped_lock buffer_mutex(buffer_mutex_);
    atoms_buffer_->WriteBytes(buffer, size);
    io_service_.post(
      boost::bind(&TcpConnect::HandleAsyncWrite, shared_from_this(), 
      atoms_buffer_, size, flag));
    return true;
  }

  bool TcpConnect::AsyncWrite(const boost::shared_ptr<ByteBuffer> bytebuffer, int flag){

    if (state_ != CS_CONNECTED){
      //LOG(ERROR) << "The socket is not opened";
      return false;
    }

    if (bytebuffer->Length() > MAX_PKT_SIZE){
      return false;
    }

    io_service_.post(
      boost::bind(&TcpConnect::HandleConstAsyncWrite, shared_from_this(),
      bytebuffer, bytebuffer->Length(), flag));
    return true;
  }

  void TcpConnect::HandleAsyncWrite(boost::shared_ptr<ByteBuffer> bytebuffer,
    int buffer_size, uint16 flag){
    if (state_ != CS_CONNECTED){
      //LOG(ERROR) << "The socket is not opened";
      return;
    }

    AppendWriteData(bytebuffer->Data(), buffer_size, flag);
    boost::mutex::scoped_lock buffer_mutex(buffer_mutex_);
    bytebuffer->Consume(buffer_size);
  }

  void TcpConnect::HandleConstAsyncWrite(const boost::shared_ptr<ByteBuffer> bytebuffer,
    int buffer_size, uint16 flag){
    if (state_ != CS_CONNECTED){
      //LOG(ERROR) << "The socket is not opened";
      return;
    }
    AppendWriteData(bytebuffer->Data(), buffer_size, flag);
  }
  void TcpConnect::AppendWriteData(const char* buffer, int size, int flag){
    BOOST_ASSERT(state_ == CS_CONNECTED);

    write_buffer_->WriteBytes(VZ_HEADER, 2);
    write_buffer_->WriteUInt16(flag);
    write_buffer_->WriteUInt32(size);
    write_buffer_->WriteBytes(buffer, size);
    WriteData();
  }

  void TcpConnect::WriteData(){

    if (is_writing_){
      //DLOG(WARNING) << "Has data wrting";
      return;
    }

    is_writing_ = true;

    int size = write_buffer_->Length();
    if (temp_size_ < size){
      temp_data_.reset(new char[size]);
      temp_size_ = size;
    }
    memcpy(temp_data_.get(), write_buffer_->Data(), size);

    socket_.async_write_some(
      boost::asio::buffer(temp_data_.get(), size),
      boost::bind(&TcpConnect::HandleDataWrite, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void TcpConnect::HandleDataWrite(const boost::system::error_code& err, int write_size){

    if (state_ == CS_CLOSED){
      return;
    }

    if (err){
      //LOG(ERROR) << err.message();
      return ConnectError(err);
    }
    is_writing_ = false;

    if (!write_buffer_->Consume(write_size)){
      //LOG(ERROR) << "Nver reach here";
      return ConnectError(boost::asio::error::invalid_argument);
    }
    if (write_buffer_->Length() > 0){
      // Continue write data
      WriteData();
    }
    else if (write_buffer_->Length() == 0){
      // //LOG(INFO) << write_size;
      SignalConnectWrite(shared_from_this());
    }
    else{
      BOOST_ASSERT(0);
    }
  }

  void TcpConnect::StartReadData(){
    BOOST_ASSERT(state_ == CS_CONNECTED);
    io_service_.post(
      boost::bind(&TcpConnect::AsyncReadData, shared_from_this()));
  }

  void TcpConnect::AsyncReadData(){
    if (state_ == CS_CLOSED){
      //LOG(WARNING) << "The socket is closed";
      return;
    }
    socket_.async_read_some(
      boost::asio::buffer(pre_read_buffer_, PRE_BUFFER_SIZE),
      boost::bind(&TcpConnect::HandleReadData, shared_from_this(),
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  void TcpConnect::HandleReadData(const boost::system::error_code& err, int read_size){

    if (state_ == CS_CLOSED){
      return;
    }

    if (err){
      //LOG(ERROR) << err.message();
      return ConnectError(err);
    }

    read_buffer_->WriteBytes(pre_read_buffer_, read_size);

    // -------------------------------------------------------------------------
    while (read_buffer_->Length() >= HEADER_BUFFERR_SIZE){
      memcpy(read_header_buffer_, read_buffer_->Data(), HEADER_BUFFERR_SIZE);
      if (read_header_buffer_[0] != VZ_HEADER[0] || read_header_buffer_[1] != VZ_HEADER[1]){
        //LOG(ERROR) << "The packet header VZ getting error";
        return ConnectError(boost::asio::error::invalid_argument);
      }
      uint16 flag = *((uint16*)(read_header_buffer_ + 2));
      uint32 packet_size = *(uint32*)(read_header_buffer_ + 4);
      flag = ntohs(flag);
      packet_size = ntohl(packet_size);
      if (packet_size > MAX_PKT_SIZE){
        //LOG(ERROR) << "The packet size large than " << MAX_PKT_SIZE;
        return ConnectError(boost::asio::error::invalid_argument);
      }
      if (packet_size <= (read_buffer_->Length() - HEADER_BUFFERR_SIZE)){
        SignalConnectRead(shared_from_this(),
          read_buffer_->Data() + HEADER_BUFFERR_SIZE, packet_size, flag);
        read_buffer_->Consume(packet_size + HEADER_BUFFERR_SIZE);
      }
      else{
        break;
      }
    }
    AsyncReadData();
  }

  void TcpConnect::CloseConnect(){

    if (state_ == CS_CLOSED){
      //DLOG(WARNING) << "The state is CS_CLOSED";
      return;
    }
    //DLOG(INFO) << "Close connect";
    state_ = CS_CLOSED;
    io_service_.post(
      boost::bind(&TcpConnect::HandleCloseConnect, shared_from_this()));
  }

  void TcpConnect::HandleCloseConnect(){

    BOOST_ASSERT(state_ == CS_CLOSED);
    if (socket_.is_open()){
      socket_.close();
    }
  }

  void TcpConnect::ConnectError(const boost::system::error_code& err){

    if (state_ == CS_CLOSED){
      return;
    }
    state_ = CS_CLOSED;
    if (socket_.is_open()){
      socket_.close();
    }
    SignalConnectError(shared_from_this(), err);
  }
}