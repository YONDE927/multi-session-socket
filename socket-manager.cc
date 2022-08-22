#include "socket-manager.h"
#include <unistd.h>

//peer_socket
peer_socket::peer_socket(int id, int fd, sockaddr_in& addr): peer_id{id}, socket_fd{fd}, peer_addr{addr}{
}

int peer_socket::get_peer_id(){
    return peer_id;
}

int peer_socket::get_socket_fd(){
    return socket_fd;
}

void peer_socket::set_socket_fd(int fd){
    socket_fd = fd;
}

sockaddr_in peer_socket::get_peer_addr(){
    return peer_addr;
}

//peer_socket_manager
peer_socket_manager::peer_socket_manager(string ip, short port){
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    self_addr.sin_port = htons(port);
    mode = 0;
    server_socket = 0;
}

int peer_socket_manager::start_server(){
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

    mode = 1;

    return server_socket;    
}

int peer_socket_manager::stop_server(){
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

//新規ペアを接続
peer_socket peer_socket_manager::accept_peer(){
    lock_guard<mutex> lock(mtx);
    peer_socket psocket;
    int tmp_socket{0};

    //クライアントソケット生成
    auto addr_size = sizeof(sockaddr_in);
    tmp_socket = accept(server_socket, (struct sockaddr*)&self_addr, (socklen_t*)&addr_size);
    if(tmp_socket < 0){
        return client_socket;
    }

    char str[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &client_addr.sin_addr, str, INET_ADDRSTRLEN);
    std::cout << "[server] accept ip " << str << " accept port " << client_addr.sin_port << std::endl;
    return client_socket;
}

int peer_socket_manager::accept_peer(peer_socket& peer){
    lock_guard<mutex> lock(mtx);
}














