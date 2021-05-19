#include<iostream>
#include<event2/event.h>
#include<string.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>
using namespace std;
void time_cb(bufferevent *be,short events,void* arg){
    cout<<"T"<<flush;
    if(events & BEV_EVENT_TIMEOUT){
        cout<<"BEV_EVENT_TIMEOUT"<<endl;
        bufferevent_free(be);
    }else if(events | BEV_EVENT_ERROR){
        cout<<"OTHERS"<<endl;
    }
}
void write_cb(bufferevent *be,void* arg){
    cout<<"W"<<flush;
}
void read_cb(bufferevent *be,void* arg){
    cout<<"R"<<endl<<flush;
    char data[1024] = {0};
    int len = bufferevent_read(be,data,sizeof(data)-1);
    cout<<"["<<data<<"]"<<endl;
    bufferevent_write(be,"OK",3);
    
}
void listen_cb(evconnlistener *ev,evutil_socket_t s,sockaddr* sin,int slen,void* arg){
    cout<<"libevent_cb"<<endl;
    event_base *base = (event_base*) arg;
    bufferevent*bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
    bufferevent_enable(bev,EV_READ|EV_WRITE);

    bufferevent_setwatermark(bev,EV_READ,10,0);
    timeval t1 = {3,0};
    bufferevent_set_timeouts(bev,&t1,0);
    bufferevent_setcb(bev,read_cb,write_cb,time_cb,base);
}
int main(){

    event_base *base = event_base_new();

    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5000);

    evconnlistener *ev= evconnlistener_new_bind(base,listen_cb,base,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,10,(sockaddr*)&sin,sizeof(sin));


    event_base_dispatch(base);
    evconnlistener_free(ev);
    event_base_free(base);
    return 0;
}