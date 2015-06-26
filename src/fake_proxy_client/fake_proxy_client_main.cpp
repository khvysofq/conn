#include "proxy_client/fake/fakeclientserver.h"
#include <boost/lexical_cast.hpp>

namespace ba = boost::asio;
namespace bs = boost::system;

typedef boost::shared_ptr<ba::ip::tcp::socket> socket_ptr;
typedef boost::weak_ptr<ba::ip::tcp::socket> socket_wptr;
typedef boost::shared_ptr<ba::io_service> io_service_ptr;

void ThreadIoServiceRun(io_service_ptr io_service){
  while (1){
    try{
      io_service->run();
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

int main(int argc, char** argv) {

  //google::InitGoogleLogging(argv[0]);
  //FLAGS_stderrthreshold = 0;
  //FLAGS_colorlogtostderr = true;

  try {
    int thread_num = 4, port = 10005;
    std::string interface_address = "0.0.0.0";

    // Read thread count from command line, if provided
    if (argc > 1)
      thread_num = boost::lexical_cast<int>(argv[1]);

    // Read port number from command line, if provided
    if (argc > 2)
      port = boost::lexical_cast<int>(argv[2]);

    // Read local interfac¡¤e address from command line, if provided
    if (argc > 3)
      interface_address = argv[3];

    ios_deque io_services;
    // See http://stackoverflow.com/questions/13219296/why-should-i-use-io-servicework
    std::deque<ba::io_service::work> io_service_work;
    io_service_ptr accpet_service(new ba::io_service);
    io_service_work.push_back(ba::io_service::work(*accpet_service));

    boost::thread_group thr_grp;

    for (int i = 0; i < thread_num; ++i) {
      // Create io server object
      io_service_ptr ios(new ba::io_service);
      // Save this io server object ot deque
      io_services.push_back(ios);
      io_service_work.push_back(ba::io_service::work(*ios));
      thr_grp.create_thread(boost::bind(ThreadIoServiceRun, ios));
    }
    // -------------------------------------------------------------------------
    // Run Firewall Manager
    // -------------------------------------------------------------------------

    proxy::FakeClientServer::Ptr server = proxy::FakeClientServer::CreateFakeClientServer(
      io_services, *accpet_service, interface_address, port, "104.151.8.38", 20003);
    server->Start();
    // Sleep at here
    accpet_service->run();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}