#include "multi-session.h"
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>

session_master::session_master(int _socket_fd): socket_fd{_socket_fd}, term{false}{
}

session_master::~session_master(){
	term = true;
	transport_th.join();
}

int session_master::sizeof_send_buffer(){
	int output{0};
	for(auto& child : children){
		output += child.second.send_buffer.size();
	}
	return output;
}

void session_master::transfer_task(){
	while(!term){
		std::unique_lock<std::mutex> buffer_lock(mtx);
		cond.wait(buffer_lock, [&](){return !term;});

		//send
		if(sizeof_send_buffer() > 0){
			for(auto& child : children){
				//send session id
				if(send(socket_fd, &child.second.session_id, sizeof(int), 0) < 0){
					std::cout << "send error" << std::endl;
					return;
				}
				//send data size
				int data_size = child.second.send_buffer.size();
				if(send(socket_fd, &data_size, sizeof(int), 0) < 0){
					std::cout << "send error" << std::endl;
					return;
				}
				//send data
				if(send(socket_fd, child.second.send_buffer.data(), data_size, 0) < 0){
					std::cout << "send error" << std::endl;
					return;
				}
				child.second.send_buffer.clear();
			}
		}

		//recv
		int rc;
		int session_id{0};
		int data_size{0};
		while(true){
			//session idだけノンブロック,来た場合は後のパケットを取得する
			//recv session id
			rc = recv(socket_fd, &session_id, sizeof(int), MSG_DONTWAIT);
			if(rc < 0){
				if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
					break;
				}else{
					std::cout << "receive error" << std::endl;
					return;	
				}
			}else if(rc == 0){
				std::cout << "receive eof" << std::endl;
				return;
			}
			//recv data size
			rc = recv(socket_fd, &data_size, sizeof(int), MSG_DONTWAIT);
			if(rc < 0){
				std::cout << "receive error" << std::endl;
				return;	
			}else if(rc == 0){
				std::cout << "receive eof" << std::endl;
				return;
			}
			//recv sizeが0以下の場合はスキップ
			if(data_size <= 0){
				continue;
			}
			//頻繁なreallocを防ぐためにcapacity()を64kbで増やす
			int recv_buffer_room = child.recv_buffer.capacity() - child.recv_buffer_cap.size();
			int recv_buffer_cap = child.recv_buffer.capacity() + data_size;
			child.recv_buffer.resize()
		}
	}
}

