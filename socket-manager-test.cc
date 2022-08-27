#include "socket-manager.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

using namespace std;

const string send_buf = "Hello";

string vector_to_string(vector& vec){
	string output(vec.begin(), vec.end());
	return output;
}

vector string_to_vector(string& str){
	vector<char> output(str.begin(), str.end());
	return output;
}

int server_task(int socket){
	if(send(socket, send_buf.c_str(), send_buf.size() + 1, 0) < 0){
		cout << "send error" << endl;
		return -1;
	}

	close(socket);
	return 0;
}

int client_task(int socket){
	string recv_buf;
	recv_buf.resize(send_buf.size() + 1);
	if(recv(socket, recv_buf.data(), recv_buf.size() + 1, 0) < 0){
		cout << "recv error" << endl;
		return -1;
	}

	cout << recv_buf << endl;

	close(socket);
	return 0;
}

int main(int argc, char** argv){
	if(argc < 2){
		cout << "not enough args" << endl;
		return 0;
	}

	string mode(argv[1]);

	socket_manager sman("127.0.0.1", 8080);

	if(mode == "client"){
		int csocket = sman.connect_server("127.0.0.1", 8080);
		if(csocket < 0){
			cout << "connect_server error" << endl;
			return 0;
		}
		if(client_task(csocket) < 0){
			cout << "client_task error" << endl;
			return 0;
		}
	}else if(mode == "server"){
		if(sman.run_server(server_task) < 0){
			cout << "start_server error" << endl;
			return 0;
		}
	}

}
