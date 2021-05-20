#include<event2/event.h>
#include<iostream>
#include<string.h>
#include<event2/buffer.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>
using namespace std;
bufferevent_filter_result filter_in(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
    char data[1024] = {0};
    int len = evbuffer_remove(s,data,sizeof(data)-1);
    cout<<"server recv"<<data<<endl;
    evbuffer_add(d,data,len);
    return BEV_OK;
}
void read_cb(bufferevent *bev,void* arg){
    cout<<"server_read_cb"<<endl;
    char data[1024] = {0};
    int len = bufferevent_read(bev,data,sizeof(data)-1);
    if(len >= 0){
        cout<<"server recv"<<data<<endl;
    }
    bufferevent_write(bev,"ok",2);
}
void write_cb(bufferevent *bev,void* arg){
    cout<<"server_write_cb"<<endl;
}
void event_cb(bufferevent *bev,short events,void* arg){
    cout<<"server_event_cb"<<endl;
}
void listen_cb(struct evconnlistener *e,evutil_socket_t s,struct sockaddr *a,int socklen,void* arg){
    cout<<"server_listen_cb"<<endl;
    event_base * base = (event_base*)arg;
    bufferevent* bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
    bufferevent *bev_fileter = bufferevent_filter_new(bev,filter_in,0,BEV_OPT_CLOSE_ON_FREE,0,0);
    bufferevent_setcb(bev_fileter,read_cb,write_cb,event_cb,nullptr);
    bufferevent_enable(bev_fileter,EV_READ|EV_WRITE);

}
void Server(event_base* base){
    cout<<"Begin Server"<<endl;
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_port=htons(5000);
    sin.sin_family = AF_INET;
    evconnlistener *ev = evconnlistener_new_bind(base,listen_cb,base,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,10,(sockaddr*)&sin,sizeof(sin));
    

}