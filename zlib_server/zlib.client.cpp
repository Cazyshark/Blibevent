#include<event2/event.h>
#include<iostream>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<event2/listener.h>
#define FILEPATHE "1.txt"
using namespace std;
bufferevent_filter_result c_filter_out(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
    cout<<"client filter out"<<endl;
    char data[1024] = {0};
    int len = evbuffer_remove(s,data,sizeof(data));
    evbuffer_add(d,data,len);
    return BEV_OK;
}
void c_read_cb(bufferevent *bev,void *arg){
    cout<<"client_read_cb"<<endl;
    char data[1024] = {0};
    int len = bufferevent_read(bev,data,sizeof(data)-1);
    if(strcmp(data,"ok")==0){
        cout<<"client data:"<<data<<endl;
        bufferevent_trigger(bev,EV_WRITE,0);
    }else{
        bufferevent_free(bev);
    }
    
   
}
void c_write_cb(bufferevent *bev,void *arg){
    FILE *fp = (FILE*)arg;

    cout<<"client_write_cb"<<endl;
    char data[1024] = {0};
    int len = fread(data,1,sizeof(data),fp);
    if(len<=0){
        fclose(fp);
        bufferevent_free(bev);
        return;
    }
    bufferevent_write(bev,data,len);
}
void c_event_cb(bufferevent *bev,short sin,void *arg){
    cout<<"client_event"<<endl;   
}
void client_event_cb(bufferevent*be, short events,void *arg){
    cout<<"client_event_cb"<<endl;
    if(events & BEV_EVENT_CONNECTED){
        cout<<"BEV_EVENT_CONNECTED"<<endl;
        bufferevent_write(be,FILEPATHE,strlen(FILEPATHE));
        bufferevent * bev_filter = bufferevent_filter_new(be,0,c_filter_out,BEV_OPT_CLOSE_ON_FREE,0,0);
        FILE* fp = fopen(FILEPATHE,"rb");
        cout<<"open file: "<<FILEPATHE<<endl;
        bufferevent_setcb(bev_filter,c_read_cb,c_write_cb,c_event_cb,fp);
        bufferevent_enable(bev_filter,EV_READ|EV_WRITE);
    }    
}
void Client(event_base* base){
    cout<<"begin Client"<<endl;
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5000);
    evutil_inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr.s_addr);
    bufferevent *bev = bufferevent_socket_new(base,-1,BEV_OPT_CLOSE_ON_FREE);
    bufferevent_enable(bev,EV_READ|EV_WRITE);
    bufferevent_setcb(bev,0,0,client_event_cb,0);
    bufferevent_socket_connect(bev,(sockaddr*)&sin,sizeof(sin));
}