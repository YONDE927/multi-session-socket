#include "socket-manager.h"
#include <iostream>
#include <unistd.h>


//peer_socket_manager
peer_socket_manager::peer_socket_manager(string ip, short port){
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    self_addr.sin_port = htons(port);
    mode = 0;
    server_socket = 0;
}

int peer_socket_manager::accept_peer_socket(){
    int tmp_socket,rc,id{0};
    int addr_size = sizeof(sockaddr_in);
    tmp_socket = accept(server_socket, (struct sockaddr*)&self_addr, (socklen_t*)&addr_size);
    if(tmp_socket < 0){
        return -1;
    }
    //peer_idの受信
    rc = recv(tmp_socket, &id, sizeof(int), 0);
    if(rc != 0){
        close(tmp_socket);
        return -1;
    }

    if(id == 0){
        //新規idを要求
        int new_id{1};
        while(true){
            if(fd_map.find(new_id) == fd_map.end()){
                //fd_mapに登録されていないid
                break;
            }
            new_id++;
        }
        id = new_id;
        //新規idを返信
        rc = send(tmp_socket, &new_id, sizeof(int), 0);
        if(rc != 0){
            close(tmp_socket);
        }
        fd_map.insert(std::make_pair(new_id, tmp_socket));
        return new_id;
    }else if(id > 0){
        //再接続を要求
        //idを返信
        rc = send(tmp_socket, &id, sizeof(int), 0);
        if(rc != 0){
            close(tmp_socket);
            return -1;
        }
        //fd_mapを更新
        fd_map.insert_or_assign(id, tmp_socket);
    }else{
        //接続エラー
        id = -1;
        rc = send(tmp_socket, &id, sizeof(int), 0);
        close(tmp_socket);
        return -1;
    }
    return id;
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

    //acceptスレッドを起動

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
int peer_socket_manager::accept_peer(shared_ptr<peer_socket> psocket){
    return 0;
}















