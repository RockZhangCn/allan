#include "thread.h"

namespace choudan{

void* StartThread(void* arg){
    //FIXME
    Thread::ThreadFunc* func = static_cast<Thread::ThreadFunc*>(arg);
    (*func)(); 
    return NULL;
}

Thread::Thread(const ThreadFunc& func)
    : started_(false),
      joined_(false),
      func_(func)

{

}

Thread::~Thread(){
   if(started_ && !joined_){
       pthread_detach(pthreadId_);
   }
}

void Thread::Start(){
    assert(!started_); 
    started_ = true; 
    
    int ret = pthread_create(&pthreadId_, NULL, &StartThread,
            &func_); 
    if(ret != 0){
        //FIXME: TODO
        assert(false);
    }
}

int Thread::Join(){
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);    
}

}
