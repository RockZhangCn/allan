
#include <unistd.h>

#include "adns_client.h"

QueryType convert(const string& type){
    QueryType t;
    if(type == "A")
        t = A;
    else if(type == "MX")
        t = MX;
    else if(type == "CNAME")
        t = CNAME;
    else if(type == "SRV")
        t = SRV;
    else if(type == "PTR")
        t = PTR;
    else
        t = A;
    return t;
}

void query(AdnsClient* ptr){
    string domain;
    string type;
    while(true){
        cout << "domain:";
        cin >> domain;
        cout << "type:";
        cin >> type;
        ptr->Query(domain, convert(type));
    }
}

int main(int argc, char* agrv[]){
    AdnsClient* clientPtr = AdnsClient::Instance();       
    clientPtr->Init();
    
    query(clientPtr);
    //clientPtr->DomainQuery("www.baidu.com");  
    //clientPtr->MXQuery("alibaba.com");  
    //clientPtr->Query("42.120.219.34", PTR);  
    //int a; 
    //cin >> a;
}
