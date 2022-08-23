#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

class somedata{
    private:
        std::mutex mtx;
        std::condition_variable cond;
        bool ready{false};
    public:
        void process(){
            std::lock_guard<std::mutex> lock(mtx);
            sleep(3); 
            ready = true;
            std::cout << "process wake wait_process" << std::endl;
            cond.notify_one();
        }

        void wait_process(){
            std::unique_lock<std::mutex> lock(mtx);
            cond.wait(lock, [this](){return ready;});
            std::cout << "wait_process wake up" << std::endl;
        }
};

int main(){
    somedata obj;
    std::thread th1(&somedata::wait_process, &obj);
    std::thread th2(&somedata::process, &obj);
    th1.join();
    th2.join();
    return 0;
}
