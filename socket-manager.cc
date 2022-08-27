#include	"socket-manager.h"
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<iostream>

//socket_manager
socket_manager::socket_manager(std::string ip, short port){
	self_addr = convert_addr(ip, port);
	server_socket = 0;
}

socket_manager::socket_manager(){}

sockaddr_in socket_manager::convert_addr(std::string& ip, short port){
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);
	return addr;	
}

int socket_manager::accpet_client(){
	int tmp_socket,rc,id{0};
	int addr_size = sizeof(sockaddr_in);
	tmp_socket = accept(server_socket, (struct sockaddr*)&self_addr, (socklen_t*)&addr_size);
	if(tmp_socket < 0){
		return -1;
	}
	return tmp_socket;
}

int socket_manager::start_server(){
	int rc{0};
	int yes{1};

	//ソケット生成
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0){
		return -1;
	}

	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes)) < 0){
		return -1;
	}

	//ポート接続
	if(bind(server_socket, (struct sockaddr*)&self_addr, sizeof(sockaddr_in)) < 0){
		return -1;
	}

	//接続待ち
	if(listen(server_socket, 10) < 0){
		return -1;
	}

	return server_socket;
}

int	socket_manager::stop_server(){
	if(server_socket > 0){
		if(shutdown(server_socket, SHUT_RDWR) < 0){
			return -1;
		}
		if(close(server_socket) < 0){
			return -1;
		}
	}
	server_socket = 0;
	return 0;
}

int socket_manager::run_server(int (*server_task)(int)){
	if(start_server() < 0){
		return -1;
	}
	while(true){
		int tmp_socket = accpet_client();
		if(tmp_socket < 0){
			continue;
		} 
		std::thread th(server_task, tmp_socket);
		th.detach();
	}
}

int socket_manager::connect_server(sockaddr_in& addr){
	int socket_fd{0};
	socklen_t socklen{sizeof(sockaddr_in)};

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		return -1;
	}

	if(connect(socket_fd, (struct sockaddr*)&addr, sizeof(sockaddr_in)) < 0){
		return -1;
	}

	return socket_fd;
}

int socket_manager::connect_server(std::string ip, short port){
	sockaddr_in addr = convert_addr(ip, port);
	return connect_server(addr);
}





















