#ifndef CHOUDAN_BASE_MUTEX_H_
#define CHOUDAN_BASE_MUTEX_H_

#include <assert.h>
#include <pthread.h>

#include <boost/noncopyable.hpp>

//referenced from Muduo/base/*
//Honor all belongs to ChenShuo

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG //make assert unavaiable

__BEGIN_DECLS //for c & c++ compile
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    __THROW __attribute__ ((__noreturn__));
    //In c __THROW is equvailent to nothing.
__END_DECLS

#endif

//_builtin_expect: gcc built-in function
#define CCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define CCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace choudan{

class Mutex: public boost::noncopyable{
    public:
        Mutex(){
            CCHECK(pthread_mutex_init(&mutex_, NULL));
        } 

        void Lock(){
            CCHECK(pthread_mutex_lock(&mutex_)); 
        }

        void Unlock(){
            CCHECK(pthread_mutex_unlock(&mutex_)); 
        }

        pthread_mutex_t* GetPthreadMutex(){
            return &mutex_;
        }

        ~Mutex(){
            CCHECK(pthread_mutex_destroy(&mutex_));
        }

    private:
        friend class Condition;
        pthread_mutex_t mutex_;
};

class MutexLock: public boost::noncopyable{
    public:
        explicit MutexLock(Mutex& mutex)
            : mutex_(mutex)
        {
            mutex_.Lock();
        }

        ~MutexLock(){
            mutex_.Unlock();
        }

    private:
        Mutex& mutex_;
};

}

#endif//CHOUDAN_BASE_MUTEX_H_
