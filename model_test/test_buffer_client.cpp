#include<iostream>
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<string.h>
using namespace std;
void client_conn_cb(bufferevent *be,short events,void* arg){
    cout<<"server time out"<<flush;
    if(events & BEV_EVENT_TIMEOUT){
        cout<<"BEV_EVENT_TIMEOUT"<<endl;
        bufferevent_free(be);
        return;
    }else if(events & BEV_EVENT_ERROR){
        cout<<"OTHERS"<<endl;
        return;
    }if (events & BEV_EVENT_CONNECTED)
    {
            cout<<"BEV_EVENT_CONNECT"<<endl;
            bufferevent_trigger(be,EV_WRITE,0)
    }
    
}
void client_write_cb(bufferevent *be,void* arg){
    cout<<"Client write"<<flush;
}
void client_read_cb(bufferevent *be,void* arg){
    cout<<"Client read"<<endl<<flush;
    FILE *fp = (FILE *)arg;
    char data[1024] = {0};
    int len = fread(data,1,sizeof(data)-1,fp);
    if(len<=0){
        fclose(fp);
        bufferevent_disable(be,EV_WRITE);
        return;
    }
    bufferevent_write(be,data,len);

    
}
// void client_listen_cb(evconnlistener *ev,evutil_socket_t s,sockaddr* sin,int slen,void* arg){
//     cout<<"libevent_cb"<<endl;
//     event_base *base = (event_base*) arg;
//     bufferevent*bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
//     bufferevent_enable(bev,EV_READ|EV_WRITE);

//     bufferevent_setwatermark(bev,EV_READ,10,0);
//     timeval t1 = {3,0};
//     bufferevent_set_timeouts(bev,&t1,0);
//     bufferevent_setcb(bev,read_cb,write_cb,time_cb,base);
// }
int main(){
    event_base* base = event_base_new();
    bufferevent *bev=bufferevent_socket_new(base,-1,BEV_OPT_CLOSE_ON_FREE);
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5000);
    evutil_inet_pton(AF_INET,"127.0.0.1",&sin.sin_addr.s_addr);
    FILE *fp = fopen("test_buffer_client.cpp","rb");
    bufferevent_setcb(bev,client_read_cb,client_write_cb,,fp);
    bufferevent_enable(bev,EV_READ|EV_WRITE);
    int re = bufferevent_socket_connect(bev,(sockaddr*)&sin,sizeof(sin));
    if(re == 0){
        cout<<"connect"<<endl;
    }
    event_base_dispatch(base);
    bufferevent_free(bev);
    event_base_free(base);
    return 0;
}