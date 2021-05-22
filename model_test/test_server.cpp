#include<iostream>
#include<event2/event.h>
#include<errno.h>
#include<string.h>
using namespace std;
#define SPORT 5000
void client_cb(evutil_socket_t s,short w,void *arg){
    cout<<"success"<<endl;
    event *ev = (event*)arg;
    if(w&EV_TIMEOUT){
        cout<<"timeout"<<flush;
         event_free(ev);
         evutil_closesocket(s);
         return;
    }
    char buff[1024] = {0};
    int len = recv(s,buff,sizeof(buff)-1,0);
    if (len > 0)
    {
        cout<<buff<<endl;
        send(s,"ok\n",2,0);
    }else{
         cout<<".."<<flush;
         event_free(ev);
         evutil_closesocket(s);
    } 
   
}
void listen_cb(evutil_socket_t s,short w,void *arg){
    cout<<"listen_cb"<<endl;
    sockaddr_in sin;
    socklen_t size = sizeof(sin);
    evutil_socket_t client = accept(s,(sockaddr*)&sin,&size);
    char ip[16] = {0};
    evutil_inet_ntop(AF_INET,&sin.sin_addr,ip,sizeof(ip)-1);
    cout<<"client ip is"<<ip<<endl;
    event_base *base = (event_base *)arg;
    event *ev = event_new(base,client,EV_READ|EV_PERSIST,client_cb,event_self_cbarg());
    timeval t = {10,0};
    cout<<"test"<<endl;
    event_add(ev,&t);
}
int main(){
    event_base *base = event_base_new();
    cout<<"test event server"<<endl;
    evutil_socket_t sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock<=0)
    {
        cout<<"socket error:"<<strerror(errno)<<endl;
        return -1;
    }
    evutil_make_socket_nonblocking(sock);
    evutil_make_listen_socket_reuseable(sock);
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port=htons(SPORT);
    int re = ::bind(sock,(sockaddr*)&sin,sizeof(sin));
    if(re!=0){
        cerr<<"bind error:"<<strerror(errno)<<endl;
        return -1;
    }
    listen(sock,10);
    event *ev = event_new(base,sock,EV_READ|EV_PERSIST,listen_cb,base);
    event_add(ev,nullptr);


    event_base_dispatch(base);
    evutil_closesocket(sock);
    event_base_free(base);
    return 0;
}