#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

using std::vector, std::map, std::pair, std::string, std::lock_guard, std::mutex, std::condition_variable, std::thread, std::shared_ptr, std::unique_ptr;

//ネットワークペアごとに一貫したソケットを提供するための機能
class peer_socket_manager{
    private:
        int mode;
        int server_socket;
        std::mutex accept_mtx;
        std::mutex waiting_mtx;
        sockaddr_in self_addr;
        map<int, int> fd_map;
        map<int, unique_ptr<condition_variable>> waiting_map;
        int peer_socket_manager::accept_peer_socket();
        peer_socket  get_peer_socket_server(int peer_id);
        peer_socket  get_peer_socket_client(int peer_id);
    public:
        struct peer_socket{
            int peer_id;
            int socket_fd;
        };
        peer_socket_manager(string ip, short port);
        int peer_socket_manager::start_server();
        peer_socket  get_peer_socket(int peer_id);
};

//memo

