#include<iostream>
#include<event2/event.h>
#include<string.h>
#include<string>
#include<event2/listener.h>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
using namespace std;
bufferevent_filter_result filter_in(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
    cout<<"filter_in"<<endl;
    char data[1024] = {0};
    int len = evbuffer_remove(s,data,sizeof(data)-1);
    for (size_t i = 0; i < len; i++)
    {
        data[i]=toupper(data[i]);
    }
    
    evbuffer_add(d,data,len);
    return BEV_OK;
}
bufferevent_filter_result filter_out(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
        cout<<"filter_out"<<endl;
        char data[1024] = {0};
    int len = evbuffer_remove(s,data,sizeof(data)-1);
    string str = "";
    str +="=========================\n";
        str+=data;
    str +="=========================\n";
    evbuffer_add(d,str.c_str(),str.size());
    return BEV_OK;
}
void read_cb(bufferevent *bev,void *arg){
    cout<<"write_cb"<<endl;
    char data[1024]={0};
    int len = bufferevent_read(bev,data,sizeof(data)-1);
    cout<<data<<endl;

    bufferevent_write(bev,data,len);
}
void write_cb(bufferevent *bev,void *arg){
    cout<<"read_cb"<<endl;
}
void event_cb(bufferevent *bev,short events,void *arg){
    cout<<"event_cb"<<endl;
}
void listen_cb(evconnlistener *ev,evutil_socket_t s,sockaddr* sin,int slen,void* arg){
   cout<<"listen_cb"<<endl;
   event_base *base = (event_base*)arg;
   bufferevent *bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
   bufferevent *bev_filter = bufferevent_filter_new(bev,filter_in,filter_out,BEV_OPT_CLOSE_ON_FREE,0,0);
   bufferevent_setcb(bev_filter,read_cb,write_cb,event_cb,ev);
   bufferevent_enable(bev_filter,EV_READ|EV_WRITE);
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