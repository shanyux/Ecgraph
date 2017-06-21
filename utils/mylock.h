#ifndef _MYLOCK_
#define _MYLOCK_
#include <mutex>
namespace ecgraph{
    class scoped_lock{
    private:
        std::mutex *mymutex;
    public:
        scoped_lock(std::mutex &mu){
           mymutex = & mu;
           mymutex->lock(); //调用线程将锁住该互斥量。
        }
        ~scoped_lock(){
            mymutex->unlock();
        }
    };

}
#endif
