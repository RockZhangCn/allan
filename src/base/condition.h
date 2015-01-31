#ifndef CHOUDAN_BASE_CONDITION_H_
#define CHOUDAN_BASE_CONDITION_H_

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include <boost/noncopyable.hpp>

#include "mutex.h"

namespace choudan{

class Condition: public boost::noncopyable{
    public:
        explicit Condition(Mutex& mutex)
            : mutex_(mutex)
        {
            CCHECK(pthread_cond_init(&cond_, NULL)); 
        }

        void Wait(){
            CCHECK(pthread_cond_wait(&cond_, mutex_.GetPthreadMutex()));
        }

    //int pthread_cond_timedwait(pthread_cond_t *restrict cond,
    //          pthread_mutex_t *restrict mutex,
    //          const struct timespec *restrict abstime);

        int Wait(uint32_t millisecond){
            struct timespec abstime;
            struct timeval tv;
            //clock_gettime(CLOCK_MONOTONIC, &abstime);
            gettimeofday(&tv, NULL);
            abstime.tv_sec = tv.tv_sec + (millisecond / 1000);
            uint32_t usec = tv.tv_usec + 
                (millisecond % 1000) * 1000;
            abstime.tv_sec += usec / 1000000;   
            usec = usec % 1000000;   

            abstime.tv_nsec = usec * 1000;    
            return pthread_cond_timedwait(&cond_,
                    mutex_.GetPthreadMutex(), &abstime);
        };

        void Notify(){
            CCHECK(pthread_cond_signal(&cond_));
        }

        ~Condition(){
            CCHECK(pthread_cond_destroy(&cond_)); 
        }
         
    private:
        Mutex& mutex_;
        pthread_cond_t cond_;
};

}//end namespace choudan

#endif//CHOUDAN_BASE_CONDITION_H_
