#include<event2/event.h>
#include<iostream>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<zlib.h>
#include<event2/listener.h>
#define FILEPATHE "1.png"
using namespace std;
struct  ClientStatus{
    FILE *fp = 0;
    bool filend = false;
    z_stream *z_output = 0;
    int readNum =0;
    int writeNum = 0;
    bool startSend = false;
    ~ClientStatus(){
        delete z_output;
        z_output = 0;
    }
};

bufferevent_filter_result c_filter_out(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
    ClientStatus *status = (ClientStatus*)arg;
    if(status->startSend=false){
        char data[1024] = {0};
        int len = evbuffer_remove(s,data,sizeof(data));
        evbuffer_add(d,data,len);
        return BEV_OK;
    }
    //start zlib
    //step 1
    evbuffer_iovec v_in[1];
    int n = evbuffer_peek(s,-1,0,v_in,1);
    if(n<=0){
        return BEV_NEED_MORE;
    }
    //step 2
    z_stream *p =status->z_output;
    if(!p){
        return BEV_ERROR;
    }
    //step 3
    p->avail_in =v_in[0].iov_len;
    p->next_in  =(Byte*)v_in[0].iov_base;
    //step 4
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(d,4096,v_out,1);
    //step 5
    p->avail_out = v_out[0].iov_len;
    p->next_out = (Byte*)v_out[0].iov_base;
    //step 6
    int re = deflate(p,Z_SYNC_FLUSH);
    if (re != Z_OK)
    {
        cerr<<"deflate failed"<<endl;
    }
    //step 7
    int nread = v_in[0].iov_len - p->avail_in;
    //step 8
    int nwrite = v_out[0].iov_len - p->avail_out;
    //step 9
    evbuffer_drain(s,nread); 
    v_out[0].iov_len = nwrite;
    evbuffer_commit_space(d,v_out,1);
    cout<<"client nread= "<<nread<<"  client nwrite= "<<nwrite<<endl;
    status->readNum += nread;
    status->writeNum += nwrite;
    //cout<<"client filter out"<<endl;
    return BEV_OK;
}
void c_read_cb(bufferevent *bev,void *arg){
   // cout<<"client_read_cb"<<endl;
    ClientStatus *status = (ClientStatus*)arg;
    char data[1024] = {0};
    int len = bufferevent_read(bev,data,sizeof(data)-1);
    if(strcmp(data,"ok")==0){
        //cout<<"client data:"<<data<<endl;
        status->startSend = true;
        bufferevent_trigger(bev,EV_WRITE,0);
    }else{
        bufferevent_free(bev);
    }
    
   
}
void c_write_cb(bufferevent *bev,void *arg){
    ClientStatus *status = (ClientStatus*)arg;
    FILE *fp = status->fp;
    if (status->filend == true)
    {   
        bufferevent *be = bufferevent_get_underlying(bev);
        evbuffer *evb = bufferevent_get_output(be);
        int len = evbuffer_get_length(evb);
        if (len <= 0)
        {
            bufferevent_free(bev);
            delete status;
            return;
        }
        return;
    }
    
    //cout<<"client_write_cb"<<endl;
    char data[1024] = {0};
    int len = fread(data,1,sizeof(data),fp);
    if(len<=0){
        cout<<"client read:"<<status->readNum<<endl;
        cout<<"Client send:"<<status->writeNum<<endl;
        fclose(fp);
        status->filend=true;
        bufferevent_flush(bev,EV_WRITE,BEV_FINISHED);
        return;
    }
    bufferevent_write(bev,data,len);
}
void c_event_cb(bufferevent *bev,short sin,void *arg){
    //cout<<"client_event"<<endl;   
}
void client_event_cb(bufferevent*be, short events,void *arg){
    //cout<<"client_event_cb"<<endl;
    if(events & BEV_EVENT_CONNECTED){
        cout<<"BEV_EVENT_CONNECTED"<<endl;
        bufferevent_write(be,FILEPATHE,strlen(FILEPATHE));
        FILE* fp = fopen(FILEPATHE,"rb");
        ClientStatus *status =new ClientStatus();
        status->fp =fp;
        status->filend = false;
        status->z_output = new z_stream();
        deflateInit(status->z_output,Z_DEFAULT_COMPRESSION);
        cout<<"open file: "<<FILEPATHE<<endl;
        bufferevent * bev_filter = bufferevent_filter_new(be,0,c_filter_out,BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS,0,status);
        bufferevent_setcb(bev_filter,c_read_cb,c_write_cb,c_event_cb,status);
        bufferevent_enable(bev_filter,EV_READ|EV_WRITE);
    }    
}
void Client(event_base* base){
    //cout<<"begin Client"<<endl;
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