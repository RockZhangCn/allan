#ifndef CHOUDAN_BASE_BLOCK_QUEUEU_H_
#define CHOUDAN_BASE_BLOCK_QUEUEU_H_

#include <stdint.h>

#include <deque>

#include <boost/noncopyable.hpp>

#include "condition.h"

using namespace std;

namespace choudan{

template<typename T>
class BlockQueue: public boost::noncopyable{
    public:

        BlockQueue()
            : mutex_(),
              cond_(mutex_), 
              queue_()
        {
        }

        uint32_t Size()const{
            MutexLock lock(mutex_);
            return queue_.size();
        }

        T Get(){
            MutexLock lock(mutex_);
            while(queue_.empty()){
                cond_.Wait();
            } 
            T t(queue_.front()); 
            queue_.pop_front();
            return t;
        }

        //@param millisecond: after wait for ms and get 
        //      nothing, return false
        bool Get(const uint32_t millisecond, T& t){
            MutexLock lock(mutex_);
            //FIXME: not strictly wait for millisecond
            if(queue_.empty()){
                cond_.Wait(millisecond);
            }

            if(queue_.empty())
                return false;

            t  = queue_.front();
            queue_.pop_front();
            return true;  
        }

        void Put(const T& item){
            MutexLock lock(mutex_);
            queue_.push_back(item);
            cond_.Notify();
        }

    private:
        mutable Mutex mutex_;
        uint32_t size_;
        Condition cond_;
        std::deque<T> queue_;
};

}//end namespace choudan
#endif//CHOUDAN_BASE_BLOCK_QUEUEU_H_
