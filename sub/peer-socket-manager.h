#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

using std::vector, std::map, std::pair, std::string, std::mutex, std::condition_variable, std::thread, std::shared_ptr;

struct peer_socket{
    int peer_id{0};
    int socket_fd{0};
};

struct peer_socket_ticket{
    //新規peerをacceptする時は、peer_idが0の条件変数をwaitする
    int peer_id{0};
    int socket_fd{0};
    bool is_new{true};
    bool is_waiting{true};
    condition_variable cond;
}
;
//ネットワークペアごとに一貫したソケットを提供するための機能
class peer_socket_manager{
    private:
        int mode{0};
        int server_socket{0};
        thread accept_thread;
        mutex waiting_mtx;
        sockaddr_in self_addr;
        map<int, shared_ptr<peer_socket_ticket>> peer_map;
        map<int, sockaddr_in> id_addr_map;
        
        int accept_socket();
        int get_new_id();
        int get_waiting_free_ticket();
        int accept_peer();
        int accept_loop();
    public:
        peer_socket_manager(string ip, short port);
        int start_server();
        int stop_server();
        peer_socket  get_peer_socket_server(int peer_id);
        peer_socket  get_peer_socket_client(int peer_id);
        peer_socket  get_peer_socket_client(sockaddr_in& addr);
};

//memo

