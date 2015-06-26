#ifndef LIBVZCONN_CONN_TCPCONNECT_H_
#define LIBVZCONN_CONN_TCPCONNECT_H_

#include "vzconn/base/basicdefines.h"
#include "vzconn/base/bytebuffer.h"
#include "boost/asio/local/stream_protocol.hpp"

namespace vzconn{

  class TcpConnect : public boost::noncopyable,
    public boost::enable_shared_from_this<TcpConnect>{
  public:

    enum ConnState{
      CS_CLOSED,
      CS_CONNECTED
    };

    typedef boost::shared_ptr<TcpConnect> Ptr;

    static TcpConnect::Ptr CreateTcpConnect( boost::asio::io_service& io_service);

    virtual ~TcpConnect();

    boost::signals2::signal<void(TcpConnect::Ptr connect,
      const boost::system::error_code& err)> SignalConnectError;

    boost::signals2::signal<void(TcpConnect::Ptr connect)> SignalConnectWrite;

    boost::signals2::signal<void(TcpConnect::Ptr connect,
      const char* buffer, int size, int flag)> SignalConnectRead;

    boost::asio::ip::tcp::socket& socket(){ return socket_; }
    boost::asio::io_service& io_service(){ return io_service_; }

    ConnState GetState() const{ return state_; }
    bool IsConnected()const { return state_ == CS_CONNECTED; }

    // The size must small than 1 MB
    virtual bool AsyncWrite(const char* buffer, int size, int flag = 0);
    bool AsyncWrite(const boost::shared_ptr<ByteBuffer> bytebuffer, int flag = 0);
    // THREAD SAFE
    // Close the connect
    virtual void CloseConnect();
  protected:
    void ConnectError(const boost::system::error_code& err);
    void StartReadData();
  private:

    friend class TcpServer;
    friend class TcpClient;

    TcpConnect(boost::asio::io_service& io_service);

    void HandleAsyncWrite(boost::shared_ptr<ByteBuffer> bytebuffer,
      int buffer_size, uint16 flag);
    void HandleConstAsyncWrite(const boost::shared_ptr<ByteBuffer> bytebuffer,
      int buffer_size, uint16 flag);
    void AppendWriteData(const char* buffer, int size, int flag);
    void WriteData();
    void HandleDataWrite(const boost::system::error_code& err, int write_size);

    void HandleCloseConnect();

    //
    void AsyncReadData();
    void HandleReadData(const boost::system::error_code& err, int read_size);

  private:
    static const int HEADER_BUFFERR_SIZE = 8;
    static const int PRE_BUFFER_SIZE = 1024 * 4;
    static const int MAX_PKT_SIZE = 1024 * 1024;
    ConnState state_;
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::scoped_array<char> temp_data_;
    int temp_size_;

    boost::mutex buffer_mutex_;
    bool is_writing_;
    boost::shared_ptr<ByteBuffer> atoms_buffer_;
    boost::scoped_ptr<ByteBuffer> write_buffer_;
    boost::scoped_ptr<ByteBuffer> read_buffer_;
    char pre_read_buffer_[PRE_BUFFER_SIZE];
    char read_header_buffer_[HEADER_BUFFERR_SIZE];

    // With client connect
    boost::asio::ip::tcp::endpoint server_addr_;
  };
}

#endif // LIBVZCONN_CONN_TCPCONNECT_H_