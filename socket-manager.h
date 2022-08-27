#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <thread>

class socket_manager{
	private:
		int server_socket{0};
		sockaddr_in self_addr;
		sockaddr_in convert_addr(std::string& ip, short port);
		int accpet_client();
	public:
		socket_manager(std::string ip, short port);
		socket_manager();
		//server
		int start_server();
		int stop_server();
		int run_server(int (*server_task)(int));
		//client
		int connect_server(sockaddr_in& addr);
		int connect_server(std::string ip, short port);
};

