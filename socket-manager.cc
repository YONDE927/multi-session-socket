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

int peer_socket_manager::accept_socket(){
    int tmp_socket,rc,id{0};
    int addr_size = sizeof(sockaddr_in);
    tmp_socket = accept(server_socket, (struct sockaddr*)&self_addr, (socklen_t*)&addr_size);
    if(tmp_socket < 0){
        return -1;
    }
    return tmp_socket;
}

int peer_socket_manager::get_new_id(){
    int id{1};
   
    while(true){
        if(peer_map.find(id) == peer_map.end()){
            break;
        }
        id++;
    }
    return id;
}

int peer_socket_manager::get_waiting_free_ticket(){
    for(auto i = peer_map.begin(); i != peer_map.end(); ++i){
        if(i->second->is_new && i->second->is_waiting){
            return i->first;
        }
    }
    return -1;
}

int peer_socket_manager::accept_peer(){
    //条件変数を扱うための排他制御
    std::lock_guard<mutex> lock(waiting_mtx);

    int tmp_socket = accept_socket();
    if(tmp_socket < 0){
        return tmp_socket;
    }
    //peer_idを受信
    int rc,id;
    if(recv(tmp_socket, &id, sizeof(int), 0) <= 0){
        close(tmp_socket);
        return -1;
    }

    //peer_mapに登録
    if(id == 0){
        //待機中で所有者のいないpeer_socket_ticketを取得
        id = get_waiting_free_ticket();
        //idを送信、idが0のときはそのままエラー送信
        if(send(tmp_socket, &id, sizeof(int), 0) <= 0){
            close(tmp_socket);
            return -1;
        }
        //更新通知
        if(id > 0){
            //待機スレッドに通知
            auto ticket = peer_map.at(id);
            ticket->socket_fd = tmp_socket;
            ticket->cond.notify_one();
        }
        return id;
    }else if(id > 0){
        //idが有効か確認
        if(peer_map.find(id) != peer_map.end()){
            //idが存在する
            //idが接続待機中なのか確認
            auto ticket = peer_map.at(id);
            if(ticket->is_waiting){
                if(send(tmp_socket, &id, sizeof(int), 0) <= 0){
                    close(tmp_socket);
                    return -1;
                }
                //ticketにソケットを登録して待機スレッドに通知
                ticket->socket_fd = tmp_socket; 
                ticket->cond.notify_one();
            }
        }
        return id;
    }

    //上の条件にあぶれた無効なidはソケットをエラー送信して切断する
    id = -1;
    if(send(tmp_socket, &id, sizeof(int), 0) <= 0){
        close(tmp_socket);
        return -1;
    }
    return id;
}

int peer_socket_manager::accept_loop(){
    int id{0};
    while(true){
        id = accept_peer();
        if(id < 0){
            std::cout << "accept fail" << std::endl;
        }
    }
}

//待機スレッド上で実行されるソケット要求関数
peer_socket peer_socket_manager::get_peer_socket_server(int peer_id){
    peer_socket psocket{0};
    if(peer_id == 0){
        auto ticket = shared_ptr<peer_socket_ticket>(new peer_socket_ticket);
        int new_id;
        //新しいticketを登録する
        {
            std::lock_guard<mutex> lock(waiting_mtx);
            new_id = get_new_id();
            ticket->peer_id = new_id;
            peer_map.insert_or_assign(new_id, ticket);
        }
        //通知を待ち、peer_socketを取り出す
        {
            std::unique_lock<mutex> lock(waiting_mtx);
            ticket->cond.wait(lock, [&](){return ticket->socket_fd > 0;});
            ticket->is_new = false;
            ticket->is_waiting = false;
            psocket.peer_id = new_id;
            psocket.socket_fd = ticket->socket_fd;
        }
    }else{
        if(peer_map.find(peer_id) == peer_map.end()){
            return psocket;
        }
        //ticketを登録する
        auto ticket = peer_map.at(peer_id);
        {
            std::lock_guard<mutex> lock(waiting_mtx);
            //ticketをリロード
            ticket->socket_fd = 0;
            ticket->is_new = false;
            ticket->is_waiting = true;
        }
        {
            std::unique_lock<mutex> lock(waiting_mtx);
            ticket->cond.wait(lock, [&](){return ticket->socket_fd > 0;});
            ticket->is_new = false;
            ticket->is_waiting = false;
            psocket.peer_id = peer_id;
            psocket.socket_fd = ticket->socket_fd;
        }
    }
    return psocket;
}

peer_socket  peer_socket_manager::get_peer_socket_client(int peer_id){
    peer_socket psocket{0};

}

//サーバー管理
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
    accept_thread = std::move(thread(&peer_socket_manager::accept_loop, this));

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
















