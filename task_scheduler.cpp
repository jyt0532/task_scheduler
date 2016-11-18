#include <iostream>
#include <thread>                                                                                                     
#include <vector>
#include <mutex>
#include <deque>
#include <queue>
//#include <time.h>
using namespace std;

struct Task{
    int activate_time;
    void (*func)();
    Task( void (*func1)(), int time): activate_time(time), func(func1){}
};
class mycomp{
    public:
        bool operator()(Task t1, Task t2){
            return t1.activate_time > t2.activate_time;
        }
};
class TaskQueue{
    public:
        TaskQueue(){}
        void push(Task t){
            unique_lock<mutex> lk(shared);
            pq.push(t);
            lk.unlock();
        }
        Task get(){
            unique_lock<mutex> lk(shared);
            Task t = pq.top();
            lk.unlock();
            return t;
        }
        void pop(){
            unique_lock<mutex> lk(shared);
            pq.pop();
            lk.unlock();
        }
        bool empty(){
            unique_lock<mutex> lk(shared);
            bool ret = pq.empty();
            lk.unlock();
            return ret;
        }
    private:
        mutex shared;
        priority_queue<Task, vector<Task>, mycomp> pq;
};
class TaskScheduler{
    public:
        TaskScheduler(){
            timer = thread([this] {checking();});
            timer.detach();
        }
        ~TaskScheduler(){
            //timer.join();
        }
        void addTask(void (*func)(), int delayed){
            time_t seconds;
            seconds = time(NULL);
            cout << seconds << endl;
            Task t(func, seconds + delayed);
            tq.push(t);
            wQ.notify_one();
        }
    private:
        thread timer;
        mutex mu;
        TaskQueue tq;
        condition_variable wQ;
        bool is_next_near(){
            time_t seconds;
            seconds = time(NULL);
            if(tq.empty()) return false;
            //cout << tq.get().activate_time << endl;
            return tq.get().activate_time - seconds < 10;
        }
        void execute(Task t){
            time_t seconds;
            seconds = time(NULL);
            if(t.activate_time < seconds){
                cout << "Action expired" << endl;
                return;
            }else{
                this_thread::sleep_for(chrono::milliseconds(1000*(t.activate_time - seconds)));    
            }
            t.func();
        }
        void checking(){
            //How to use condition variable
            /*
            while(1){
                unique_lock<mutex> lk(mu);
                wQ.wait(lk, bind(&TaskScheduler::is_next_near, this));
                Task nT = tq.get();
                tq.pop();
                thread t = thread(&TaskScheduler::execute, this, nT);
                t.detach();
                lk.unlock();
            }*/
            while(1){
                while(!is_next_near()){
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
                unique_lock<mutex> lk(mu);
                Task nT = tq.get();
                tq.pop();
                thread t = thread(&TaskScheduler::execute, this, nT);
                t.detach();
                lk.unlock();
            }
        }
};
void func1(){
    cout << "Excuting func1" << endl;
}
void func2(){
    cout << "Excuting func2" << endl;
}
void func3(){
    cout << "Excuting func3" << endl;
}
void func4(){
    cout << "Excuting func4" << endl;
}
void func5(){
    cout << "Excuting func5" << endl;
}

int main(){
    TaskScheduler ts;
    ts.addTask(func1, 3);
    ts.addTask(func2, 3);
    ts.addTask(func3, 5);
    ts.addTask(func4, 10);
    this_thread::sleep_for(chrono::milliseconds(4000));
    ts.addTask(func5, 3);
    this_thread::sleep_for(chrono::milliseconds(20000));
    return 0;
}
