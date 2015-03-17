
#include <stdlib.h>

#include <vector>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "base/thread.h"
#include "base/block_queue.h"

using namespace std;
using namespace choudan;

class WorkManager{
    public:
        WorkManager(int consumerNum, int producerNum, int num)
            : numbers_(num)
              //stop_(false)
        {
            consumerThread_.reserve(consumerNum);
            for(int i = 0; i < consumerNum; ++i){
                ThreadPtr ptr(new Thread(boost::bind(&WorkManager::Consume, this))); 
                ptr->Start();
                consumerThread_.push_back(ptr);
            } 
            producerThread_.reserve(producerNum);
            for(int i = 0; i < producerNum; ++i){
                ThreadPtr ptr(new Thread(boost::bind(&WorkManager::Produce, this, num))); 
                ptr->Start();
                producerThread_.push_back(ptr);
            } 
        }

        ~WorkManager(){
            for(uint32_t i = 0; i < producerThread_.size(); ++i)  
                producerThread_[i]->Join();

            for(uint32_t i = 0; i < consumerThread_.size(); ++i)  
                blockQueue_.Put(-1);

            for(uint32_t i = 0; i < consumerThread_.size(); ++i)  
                consumerThread_[i]->Join();
        }
        
    private:

        void Produce(int num){
            for(int i = 0; i < num; ++i){
                blockQueue_.Put(i);
            }
        }

        void Consume(){
            while(true){
                if(blockQueue_.Get() < 0)
                    break;
            }
        }

        int numbers_;
        vector<ThreadPtr> consumerThread_;
        vector<ThreadPtr> producerThread_;
        BlockQueue<int> blockQueue_;
};

int main(int argc, char* argv[]){
    if(argc != 4){
        cout << "Usage: " << argv[0] << " consumerNum producerNum numbers" << endl;
        return 1;
    }
    cout << "----------------------------------" << endl;
    cout << "consumer:" << atoi(argv[1]) << endl;  
    cout << "producer:" << atoi(argv[2]) << endl;  
    cout << "numbers :" << atoi(argv[3]) << endl;  

    WorkManager work(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
}
