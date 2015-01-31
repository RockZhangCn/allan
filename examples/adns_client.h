#ifndef CHOUDAN_EXAMPLES_ADNS_CLIENT_H_
#define CHOUDAN_EXAMPLES_ADNS_CLIENT_H_

#include <map>
#include <vector>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "adns.h"

#include "base/thread.h"
#include "base/block_queue.h"

using namespace choudan;

enum QueryType{
    A = 0, 
    MX,   
    CNAME,
    SRV,
    PTR
};

class AdnsClient: public boost::noncopyable{
    public: 
        typedef boost::function<void (const vector<string>&)> Callback;

        static AdnsClient* Instance(); 
        void Init();

        void DomainQuery(const string& object);
        void PTRQuery(const string& object);
        void MXQuery(const string& object);
        void Query(const string& object, QueryType type);
        void Query(const string& object, QueryType type, 
                Callback func);

        void RegisterCallback(Callback& func){
            defaultCallback_ = func;
        }

        ~AdnsClient();

    private:

        AdnsClient();

        adns_rrtype TypeConvert(QueryType type);

        struct QueryEntry{

            QueryEntry(const string& object, adns_rrtype type)
                : object_(object), type_(type){}

            QueryEntry(const string& object, adns_rrtype type, 
                    Callback func)
                : object_(object), type_(type), func_(func){}

            string object_;
            adns_rrtype type_;
            Callback func_;
        };

        typedef boost::shared_ptr<QueryEntry> QueryEntryPtr;

        void DoQuery(const QueryEntryPtr& entryPtr);
        void WorkLoop();
        bool SubmitQuery(adns_state ads, 
                const QueryEntryPtr& query);

        void HandleAnswer(adns_answer *answer);
        void HandleNewQuery(adns_state ads, int32_t& unfinish);


        typedef pair<string, QueryEntryPtr> QueryPair;
        typedef map<string, QueryEntryPtr> QueryMap;
        typedef QueryMap::iterator QueryMapItr;
        QueryMap queryMap_;

        bool stop_;
        bool started_;

        BlockQueue<QueryEntryPtr> blockQueue_;

        Callback defaultCallback_;
        ThreadPtr threadPtr_;
        Mutex mutex_;
        //static AdnsClient* inst_;
};
//typedef boost::shared_ptr<AdnsClient> AndsClientPtr;

#endif//CHOUDAN_EXAMPLES_ADNS_CLIENT_H_
