#include <boost/program_options.hpp>
#include "udp_client_server.h"
#include <chrono>
#include <thread>

using namespace boost::program_options;

struct sync_request {
  uint8_t msgID;
  uint8_t msgContent;
};

struct color_request {
  uint8_t msgID;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};


void run_multicast()
{
  //udp_server server("192.168.2.122",7001);
  std::vector<udp_client*> client_list;
  //client_list.push_back(std::make_unique<udp_client>(udp_client("192.168.2.122",7001)));
  //client_list.push_back(udp_client("192.168.2.122",7001));
  //auto client_test = std::make_unique<udp_client>(udp_client("192.168.2.122",7001));
  udp_client* client_0 = new udp_client("239.1.2.3",7001);
  client_list.push_back(client_0);

  std::cout << "Created Client list" << std::endl;

  udp_server server("192.168.2.120",7001);
  std::cout << "Created Server" << std::endl;

  sync_request msg;
  msg.msgID = 0x02;

  uint8_t response = 0;

  /* Send messages */
  std::cout << "Sending first sync message" << std::endl;
  msg.msgContent = 0xEA;
  client_list[0]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void stop_multicast()
{
  //udp_server server("192.168.2.122",7001);
  std::vector<udp_client*> client_list;
  //client_list.push_back(std::make_unique<udp_client>(udp_client("192.168.2.122",7001)));
  //client_list.push_back(udp_client("192.168.2.122",7001));
  //auto client_test = std::make_unique<udp_client>(udp_client("192.168.2.122",7001));
  udp_client* client_0 = new udp_client("239.1.2.3",7001);
  client_list.push_back(client_0);

  std::cout << "Created Client list" << std::endl;

  udp_server server("192.168.2.120",7001);
  std::cout << "Created Server" << std::endl;

  sync_request msg;
  msg.msgID = 0x03;

  uint8_t response = 0;

  /* Send messages */
  std::cout << "Sending first sync message" << std::endl;
  msg.msgContent = 0xEA;
  client_list[0]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void run_sync()
{
  //udp_server server("192.168.2.122",7001);
  std::vector<udp_client*> client_list;
  //client_list.push_back(std::make_unique<udp_client>(udp_client("192.168.2.122",7001)));
  //client_list.push_back(udp_client("192.168.2.122",7001));
  //auto client_test = std::make_unique<udp_client>(udp_client("192.168.2.122",7001));
  udp_client* client_0 = new udp_client("192.168.2.124",7001);
  udp_client* client_1 = new udp_client("192.168.2.122",7001);
  udp_client* client_2 = new udp_client("192.168.2.113",7001);
  client_list.push_back(client_0);
  client_list.push_back(client_1);
  client_list.push_back(client_2);

  std::cout << "Created Client list" << std::endl;

  udp_server server("192.168.2.120",7001);
  std::cout << "Created Server" << std::endl;

  sync_request msg;
  msg.msgID = 0x02;

  uint8_t response = 0;

  /* Send messages */
  std::cout << "Sending first sync message" << std::endl;
  msg.msgContent = 0xEA;
  client_list[0]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << "Sending second sync message" << std::endl;
  msg.msgContent = 0x65;
  client_list[1]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << "Sending third sync message" << std::endl;
  msg.msgContent = 0x00;
  client_list[2]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  /* Wait for feedbacks */
  int retval = server.timed_recv((char*)&response,sizeof(response),500);
  std::cout << "Response " << (uint8_t)response << std::endl;
  if(retval == -1)
  {
    std::cout << "Receive timeout " << std::endl;
  }

  retval = server.timed_recv((char*)&response,sizeof(response),1000);
  std::cout << "Response " << (uint8_t)response << std::endl;
  if(retval == -1)
  {
    std::cout << "Receive timeout " << std::endl;
  }

  retval = server.timed_recv((char*)&response,sizeof(response),1500);
  std::cout << "Response " << (uint8_t)response << std::endl;
  if(retval == -1)
  {
    std::cout << "Receive timeout " << std::endl;
  }


  delete client_0;
  delete client_1;
  delete client_2;
}

void run_test_multicast()
{

  //udp_server server("192.168.2.122",7001);
  std::vector<udp_client*> client_list;
  //client_list.push_back(std::make_unique<udp_client>(udp_client("192.168.2.122",7001)));
  //client_list.push_back(udp_client("192.168.2.122",7001));
  //auto client_test = std::make_unique<udp_client>(udp_client("192.168.2.122",7001));
  udp_client* client_0 = new udp_client("239.1.2.3",7001);
  client_list.push_back(client_0);

  std::cout << "Created Client list" << std::endl;

  udp_server server("192.168.2.120",7001);
  std::cout << "Created Server" << std::endl;

  /* Sync request */
  color_request msg;
  msg.msgID = 0x02;

  uint8_t response = 0;

  /* Send messages */
  std::cout << "Sending first sync message" << std::endl;

  client_list[0]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  msg.msgID = 0x01;
  uint32_t num_iterations = 100;

  std::cout << "Starting streaming with " << num_iterations << " iterations" << std::endl;

  uint8_t count_R = 0;
  uint8_t count_G = 0;
  uint8_t count_B = 0;

  /* Send messages */
  while(num_iterations > 0)
  {

    std::cout << "Iteration " << num_iterations << std::endl;
    num_iterations--;

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.red = msg.red << 1;
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(27));
    }

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.green = msg.green << 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(27));
    }

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.blue = msg.blue << 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(27));
    }

  }

  std::cout << "Finishing streaming" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  /* Ending streaming mode */
  msg.msgID = 0x03;
  client_list[0]->send((char*)&msg, sizeof(msg));

  delete client_0;

}



void run_test()
{

  /* Do synchronization */
  //run_sync();

  std::cout << "Synchronization done. Waiting 1 second" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  //udp_server server("192.168.2.122",7001);
  std::vector<udp_client*> client_list;
  //client_list.push_back(std::make_unique<udp_client>(udp_client("192.168.2.122",7001)));
  //client_list.push_back(udp_client("192.168.2.122",7001));
  //auto client_test = std::make_unique<udp_client>(udp_client("192.168.2.122",7001));
  udp_client* client_0 = new udp_client("192.168.2.106",7001);
  udp_client* client_1 = new udp_client("192.168.2.111",7001);
  udp_client* client_2 = new udp_client("192.168.2.113",7001);
  client_list.push_back(client_0);
  client_list.push_back(client_1);
  client_list.push_back(client_2);

  std::cout << "Created Client list" << std::endl;

  udp_server server("192.168.2.120",7001);
  std::cout << "Created Server" << std::endl;

  color_request msg;
  msg.msgID = 0x01;

  uint32_t num_iterations = 100;

  std::cout << "Starting streaming with " << num_iterations << " iterations" << std::endl;

  uint8_t count_R = 0;
  uint8_t count_G = 0;
  uint8_t count_B = 0;

  /* Send messages */
  while(num_iterations > 0)
  {

    std::cout << "Iteration " << num_iterations << std::endl;
    num_iterations--;

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.red = msg.red << 1;
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[1]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[2]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.green = msg.green << 1;
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[1]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[2]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }

    msg.red = 0x01;
    msg.green = 0x01;
    msg.blue = 0x01;

    for(uint8_t red_count = 0; red_count < 7; red_count++)
    {
      msg.blue = msg.blue << 1;
      client_list[0]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[1]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      client_list[2]->send((char*)&msg, sizeof(msg));
      std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }

  }

  std::cout << "Finishing streaming" << std::endl;

  /* Ending streaming mode */
  msg.msgID = 0x03;
  client_list[0]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(12));

  client_list[1]->send((char*)&msg, sizeof(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(12));

  client_list[2]->send((char*)&msg, sizeof(msg));


  delete client_0;
  delete client_1;
  delete client_2;
}



int main (int argc, const char *argv[])
{

  std::string text_given;
  uint32_t num_iterations;
  uint8_t do_sync = 0;

  //Parse command line parameters
  options_description desc{"Options"};
  desc.add_options()
    ("help,h", "Help screen")
    ("iterations,i", value<uint32_t>(), "Number of iterations")
    ("execution,e", "execution")
    ("execution_multicast,a", "execution_multicast")
    ("multicast,m", "multicast")
    ("terminate,t", "terminate")
    ("sync,s", "Number of iterations");

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);

  //Show help
  if (vm.count("help"))
  {
    std::cout << "Help" << '\n';
  }

  if (vm.count("iterations"))
  {
    num_iterations = vm["iterations"].as<uint32_t>();
    std::cout << "Number of UDP iterations: " << num_iterations << std::endl;
  }
  else if (vm.count("sync"))
  {
    run_sync();
    std::cout << "Doing synchronization" << std::endl;
  }
  else if (vm.count("execution"))
  {
    std::cout << "Running communication test" << std::endl;
    run_test();
  }
  else if (vm.count("multicast"))
  {
    std::cout << "Running multicast test" << std::endl;
    run_multicast();
  }
  else if (vm.count("terminate"))
  {
    std::cout << "Stopping multicast test" << std::endl;
    stop_multicast();
  }
  else if (vm.count("execution_multicast"))
  {
    std::cout << "Starting multicast sequence" << std::endl;
    run_test_multicast();
  }




  //std::cout << "Received response: " << (uint8_t)response << std::endl;
  //std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;

  return 0;
}
