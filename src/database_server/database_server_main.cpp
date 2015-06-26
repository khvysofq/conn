#include "database_server_defines.h"
#include "database_session.h"

int main(int argc, char* argv[]){

  boost::asio::io_service io_service_;
  database::DatabaseSession database_session(io_service_);
  database_session.ConnectDatabase("tcp://115.28.128.168:3306", "root", "khvysofq");
  //database_session.InsertProxyServer("www.youtube.com", 123452685, 20003);
  database_session.UpdateProxyServerPing(123456789);
  database_session.GetAllProxyServer();
  return 0;
}