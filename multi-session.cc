#include "multi-session.h"
#include <iostream>
#include <memory>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>

session_master::session_master(int _socket_fd): socket_fd{_socket_fd}, term{false}{
}

session_master::~session_master(){
	stop();
}

int session_master::sizeof_send_buffer(){
	int output{0};
	for(auto& child : children){
		output += child.second.send_buffer.size();
	}
	return output;
}

void session_master::send_task(){
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
	}
}

void session_master::recv_task(){
	//recv
	int rc;
	int session_id{0};
	int data_size{0};
	while(!term){
		std::lock_guard<std::mutex> lock(mtx);
		//session idだけノンブロック,来た場合は後のパケットを取得する
		//recv session id
		rc = recv(socket_fd, &session_id, sizeof(int), 0);
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
		rc = recv(socket_fd, &data_size, sizeof(int), 0);
		if(rc < 0){
			std::cout << "receive error" << std::endl;
			return;	
		}else if(rc == 0){
			std::cout << "receive eof" << std::endl;
			return;
		}
		//recv sizeが0以下の場合はスキップ
		if(data_size == 0){
			continue;
		}else if(data_size < 0){
			//delete_session
			delete_session(session_id);
			continue;
		}

		//不正session_id
		if(children.find(session_id) == children.end()){
			std::shared_ptr<char> buffer(new char[data_size]);
			rc = recv(socket_fd, buffer.get(), data_size, 0);
			if(rc < 0){
				std::cout << "receive error" << std::endl;
				return;	
			}else if(rc == 0){
				std::cout << "receive eof" << std::endl;
				return;
			}
		}else{
			//capacityの変更
			auto& child = children.at(session_id);
			const int add_cap_size = 65536;
			int recv_buffer_room = child.recv_buffer.capacity() - child.recv_buffer.size();
			if(recv_buffer_room < data_size){
				int room_lack = data_size - recv_buffer_room;
				int add_num = room_lack / add_cap_size + 1;
				child.recv_buffer.reserve(child.recv_buffer.capacity() + add_cap_size * add_num);
			}
			//recv data
			child.recv_buffer.resize(child.recv_buffer.size() + data_size);
			rc = recv(socket_fd, child.recv_buffer.data() + child.recv_buffer.size(), data_size, 0);
			if(rc < 0){
				std::cout << "receive error" << std::endl;
				return;	
			}else if(rc == 0){
				std::cout << "receive eof" << std::endl;
				return;
			}
			//notify child
			child.cond.notify_one();
		}
	}
}

void session_master::run(){

}

void session_master::stop(){

}
