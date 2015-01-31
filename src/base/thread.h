#ifndef CHOUDAN_BASE_THREAD_H_
#define CHOUDAN_BASE_THREAD_H_

#include <pthread.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace choudan{

class Thread: public boost::noncopyable{
    public: 
        typedef boost::function<void ()> ThreadFunc;
        explicit Thread(const ThreadFunc& func);
        ~Thread();

        void Start();
        int  Join();

    private:

        bool started_;
        bool joined_;
        pthread_t  pthreadId_;
        ThreadFunc func_; 
};

typedef boost::shared_ptr<Thread> ThreadPtr;

}//end namespace choudan
#endif//CHOUDAN_BASE_THREAD_H_
