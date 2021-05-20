#include<event2/event.h>
#include<iostream>
#include<string.h>
#include<string>
#include<zlib.h>
#include<event2/buffer.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>
using namespace std;
struct Status
{
   bool start = false;
   FILE *fp = 0;
   z_stream* p;
   int recvNum  = 0;
   int writeNum = 0;
};

bufferevent_filter_result filter_in(evbuffer *s,evbuffer *d,ev_ssize_t limit,bufferevent_flush_mode mode,void* arg){
    Status *status = (Status*) arg;
    if(!status->start){
        char data[1024] = {0};
        int len = evbuffer_remove(s,data,sizeof(data)-1);
        cout<<"server recv:"<<len<<endl;
        evbuffer_add(d,data,len);
        return BEV_OK;
    }
    evbuffer_iovec v_in[1];
    int n = evbuffer_peek(s,-1,nullptr,v_in,1);
    if(n<=0){
        return BEV_NEED_MORE;
    }
    z_stream *p =status->p;
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
    int re = inflate(p,Z_SYNC_FLUSH);
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
    //cout<<"server nread= "<<nread<<"  server nwrite= "<<nwrite<<endl;
    status->recvNum += nread;
    status->writeNum +=  nwrite;
    return BEV_OK;
}

void read_cb(bufferevent *bev,void* arg){
    Status *status = (Status*) arg;

    if(status->start==false){
        char data[1024] ={0};
        bufferevent_read(bev,data,sizeof(data)-1);
        string out = "out/";
        out += data;
        status->fp =  fopen(out.c_str(),"wb");
        bufferevent_write(bev,"ok",2);
        status->start = true;
        return;
    }
    do{
        char data[1024] = {0};
        int len = bufferevent_read(bev,data,sizeof(data));
        if(len>=0){
            fwrite(data,1,len,status->fp);
            fflush(status->fp);
        }
    }while(evbuffer_get_length(bufferevent_get_input(bev))>0);
    
    
}
void write_cb(bufferevent *bev,void* arg){
    //cout<<"server_write_cb"<<endl;
}
void event_cb(bufferevent *bev,short events,void* arg){
    cout<<"server_event_cb"<<endl;
    Status *status = (Status*)arg;
    if(events & BEV_EVENT_EOF){
        cout<<"server event BEV_EVENT_EOF"<<endl;
        if(status->fp){
            cout<<"Server recv:"<<status->recvNum<<endl;
            cout<<"Server write:"<<status->writeNum<<endl;
            fclose(status->fp);
            status->fp = 0;
        }
        bufferevent_free(bev);
    }
}
void listen_cb(struct evconnlistener *e,evutil_socket_t s,struct sockaddr *a,int socklen,void* arg){
    //out<<"server_listen_cb"<<endl;
    event_base * base = (event_base*)arg;
    bufferevent* bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
    Status *status = new Status();
    status->p = new z_stream();
    inflateInit(status->p);
    bufferevent *bev_fileter = bufferevent_filter_new(bev,filter_in,0,BEV_OPT_CLOSE_ON_FREE,0,status);
    bufferevent_setcb(bev_fileter,read_cb,write_cb,event_cb,status);
    bufferevent_enable(bev_fileter,EV_READ|EV_WRITE);

}
void Server(event_base* base){
    //cout<<"Begin Server"<<endl;
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_port=htons(5000);
    sin.sin_family = AF_INET;
    evconnlistener *ev = evconnlistener_new_bind(base,listen_cb,base,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,10,(sockaddr*)&sin,sizeof(sin));
    

}