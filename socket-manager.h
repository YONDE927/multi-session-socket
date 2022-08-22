#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>

using std::vector, std::map, std::pair, std::string, std::lock_guard, std::mutex, std::thread;

//ネットワークペアごとに一貫したソケットを提供するための機能
class peer_socket{
    private:
        int peer_id;
        int socket_fd;
        mutex mtx;
        sockaddr_in peer_addr;
    public:
        peer_socket();
        peer_socket(sockaddr_in& addr);
        peer_socket(int id, int fd, sockaddr_in& addr);
        int get_peer_id();
        int get_socket_fd();
        void set_socket_fd(int fd);
        sockaddr_in get_peer_addr();
};

class peer_socket_manager{
    private:
        int mode;
        int server_socket;
        thread accept_thread; 
        sockaddr_in self_addr;
        map<int, int> peer_map;
    public:
        peer_socket_manager(string ip, short port);

        //accept_threadを立ち上げる
        int start_server();
        int stop_server();

        //新たなセッションもしくは既存のセッションを受け付ける
        int accept_peer(peer_socket& peer);

        peer_socket connect_peer(sockaddr_in& peer_addr);
        int reconnect_peer(peer_socket& peer);

        void free_peer(peer_socket& peer);
};

//memo
//排他制御を駆使して、acceptと既存リストのロックが様々なaccept_peerで競合しないように頑張る。
//
//
//

