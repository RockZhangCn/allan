#include "adns_client.h"

#include <errno.h>
#include <stdio.h>

#include <boost/bind.hpp>

#include <iterator>

#include "base/types.h"

//AdnsClient* AdnsClient::inst_ = NULL;

const uint32_t MAX_GET_ONE_TIME  = 10;
const uint32_t MAX_GET_WITH_NO_FINISH = MAX_GET_ONE_TIME / 2;

void DefaultCallBack(const vector<string>& results){
    copy(results.begin(), results.end(), 
            ostream_iterator<string>(cout, "\n")); 
}

AdnsClient* AdnsClient::Instance(){
    static AdnsClient inst;
    return &inst;
}

AdnsClient::AdnsClient()
    : stop_(false),
      started_(false)
{
}

void AdnsClient::Init(){
    if(!started_){
        threadPtr_.reset(new Thread(boost::bind(
                &AdnsClient::WorkLoop, this)));
        threadPtr_->Start();
        started_ = true;
    }
}

AdnsClient::~AdnsClient(){
    if(threadPtr_){
        stop_ = true;
        threadPtr_->Join();
    }
}

void AdnsClient::DomainQuery(const string& object){
    QueryEntryPtr entryPtr(new QueryEntry(
        object.c_str(), adns_r_addr, DefaultCallBack));       
    DoQuery(entryPtr); 
}

void AdnsClient::MXQuery(const string& object){
    QueryEntryPtr entryPtr(new QueryEntry(
        object.c_str(), adns_r_mx, &DefaultCallBack));       
    DoQuery(entryPtr); 
}

void AdnsClient::PTRQuery(const string& object){
    QueryEntryPtr entryPtr(new QueryEntry(
        object.c_str(), adns_r_ptr, &DefaultCallBack));       
    DoQuery(entryPtr); 
}

void AdnsClient::Query(const string& object, QueryType type){
    QueryEntryPtr entryPtr(new QueryEntry(
        object.c_str(), TypeConvert(type), &DefaultCallBack));       
    DoQuery(entryPtr); 
}

void AdnsClient::Query(const string& object, QueryType type,
       Callback func){
    QueryEntryPtr entryPtr(new QueryEntry(
        object.c_str(), TypeConvert(type), func));       
    DoQuery(entryPtr); 
}

void AdnsClient::DoQuery(const QueryEntryPtr& entryPtr){
    blockQueue_.Put(entryPtr);   
    MutexLock lock(mutex_);
    queryMap_.insert(QueryPair(entryPtr->object_,
                entryPtr));  
}

adns_rrtype AdnsClient::TypeConvert(QueryType type){
    adns_rrtype t;
    switch(type){
    case A:
        t = adns_r_a;
        break;
    case MX:
        t = adns_r_mx;
        break;
    case CNAME:
        t = adns_r_cname;
        break;
    case SRV:
        t = adns_r_srv;
        break;
    case PTR:
        t = adns_r_ptr;
        break;
    default:
        t = adns_r_none;
    }
    return t;
}

bool AdnsClient::SubmitQuery(adns_state ads, 
        const QueryEntryPtr& entryPtr){
    //Note: first, just use adns_qf_want_allaf flag
    adns_query qu;
    int quflags = adns_qf_want_allaf | adns_qf_owner; 
    int err;

    if(entryPtr->type_ == adns_r_ptr || 
            entryPtr->type_ == adns_r_ptr_raw){
        adns_rr_addr a;   
        a.len = sizeof(a.addr);
        if(adns_text2addr(entryPtr->object_.c_str(), 
                0, adns_qf_addrlit_scope_forbid, &a.addr.sa, 
                //implicit_cast<socklen_t*>(static_cast<void*>(&a.len))) == EINVAL){
                static_cast<socklen_t*>(static_cast<void*>(&a.len))) == EINVAL){
            cout << "Invalid Ip address" << endl;
            return false;
        }

        err = adns_submit_reverse(ads, &a.addr.sa, 
                entryPtr->type_,
                static_cast<adns_queryflags>(quflags), 
                NULL, &qu); 
    }else{
        err = adns_submit(ads, entryPtr->object_.c_str(), 
            entryPtr->type_, 
            static_cast<adns_queryflags>(quflags), NULL, &qu);
    }
    if(err){
        perror("adns_submit");
        return false;
    }
    return true;
}

void AdnsClient::HandleAnswer(adns_answer *answer){
    if(!answer->owner){
        cout << "Not expected answer with no owner!" << endl;
        free(answer);
        return;
    }

    if(!answer->nrrs){
        cout << "owner:" << answer->owner << " "
            << adns_strerror(answer->status) << endl;  
        if(answer->cname){
            cout << "CNAME:" << answer->cname << endl;
        }
        free(answer);
        return;
    }

    cout << "owner: " <<  answer->owner << endl;
    QueryMapItr it = queryMap_.find(answer->owner);  
    if(it == queryMap_.end()){
        //ip.in-addr.arpa
        int len = static_cast<char*>(memchr(answer->owner, 
            'i', strlen(answer->owner))) - answer->owner - 1; 
        if(len > 6){
            string tmp(answer->owner, len);
            string origin;
            size_t start = 0, end = 0;
            while(true){
                end = tmp.find(".", start);
                if(end == string::npos){
                    origin = string(tmp, start, tmp.size() - start) + origin;
                    break;
                }
                origin = "." + string(tmp, start, end - start) + origin; 
                start = end + 1;
            }
            it = queryMap_.find(origin);
        }
    }
    assert(it != queryMap_.end());
    adns_rrtype type = answer->type;
    //const typeinfo* typei = NULL;
    //typei = adns__findtype(type);
    vector<string> results;
    int nrrs = answer->nrrs, rrn;
    const char* rrp;
    char* datastr;
    for(rrn = 0, rrp = static_cast<const char *>(answer->rrs.untyped); 
            rrn < nrrs; rrn++, rrp += answer->rrsz){
        adns_status st = adns_rr_info(type, 
                NULL, NULL, NULL, rrp, &datastr);
        if(st == adns_s_nomemory){
            cout << "out of memory" << endl; 
            return;
        }
        results.push_back(string(datastr));
        free(datastr);
    }
    //handle result
    it->second->func_(results);
    free(answer);
}

void AdnsClient::HandleNewQuery(adns_state ads, 
        int32_t& unfinish){
    int maxGet = 0, max = blockQueue_.Size(); 
    if(unfinish != 0){
        maxGet = MAX_GET_WITH_NO_FINISH;
    }else{
        max = max == 0? 1 : max;
        maxGet = MAX_GET_ONE_TIME;
    }
    max = max > maxGet ? maxGet : max;
    for(int i = 0; i < max; ++i){
        QueryEntryPtr queryPtr = blockQueue_.Get();
        if(SubmitQuery(ads, queryPtr)){
            ++unfinish;
        }
        //else{
        //    blockQueue_.Put(queryPtr);
        //}
    } 
}

void AdnsClient::WorkLoop(){
    adns_state ads;
    adns_answer *answer;
    void *qun_v;

    struct timeval *tv, tvbuf;
    int r, maxfd, unfinish = 0; 
    fd_set readfds, writefds, exceptfds;

    adns_initflags flags = adns_if_nosigpipe;
    r = adns_init(&ads, flags, 0); // 0 means stderr
    while(!stop_){
        while(true){
            adns_query qu = NULL;
            r = adns_check(ads, &qu, &answer, &qun_v);
            if(r == EAGAIN)// not done
                break;
            if(r == ESRCH){// means no request
                assert(unfinish == 0);
                break;
            }
            //Callback 
            HandleAnswer(answer);
            --unfinish;
        }
        
        HandleNewQuery(ads, unfinish);

        maxfd= 0;
        FD_ZERO(&readfds); 
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
        adns_beforeselect(ads, &maxfd, &readfds, &writefds,
            &exceptfds, &tv,&tvbuf, 0);   
        r = select(maxfd, &readfds, &writefds, &exceptfds,
                    tv); 
        if(r == -1){
            if(errno == EINTR)
                continue;
            //FIXME
        }
        adns_afterselect(ads, maxfd, &readfds, &writefds,
                &exceptfds, 0);
    }
}
