#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

struct session_child{
	int session_id;
	std::vector<char> send_buffer;
	std::vector<char> recv_buffer;
	std::condition_variable cond;
};

class session_master{
	private:
		int socket_fd;
		bool term;
		std::map<int, session_child> children;
		std::thread recv_th;
		std::thread send_th;
		std::mutex mtx;
		std::condition_variable cond;

		int sizeof_send_buffer();
		int sizeof_recv_buffer();
		void send_task();
		void recv_task();
	public:
		session_master(int _socket_fd);
		~session_master();
		void run();
		void stop();
		int new_session();
		int delete_session(int session_id);
		int send_data(int session_id, const std::vector<char>& buffer);
		int recv_data(int session_id, std::vector<char>& buffer);
};

//これいらないじゃないか
//始祖ファイルさえ一つのソケットで創ってしまえば問題ないのでは？
