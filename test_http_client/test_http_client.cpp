#include<event2/event.h>
#include<event2/listener.h>
#include<event2/http.h>
#include<event2/keyvalq_struct.h>
#include<string.h>
#include<string>
#include<strings.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<iostream>
using namespace std;
void http_client(evhttp_request *req,void* ctx){
    cout<<"http_client"<<endl;
    bufferevent *bev = (bufferevent *)ctx;
    if(req == nullptr){
        int errcode = EVUTIL_SOCKET_ERROR();
        cout<<"socket error"<<evutil_socket_error_to_string(errcode);
        return;
    }
    const char *path = evhttp_request_get_uri(req);
    cout<<"requset path is"<<path<<endl;
    string filepath = ".";
    filepath += path;
    cout<<"Response: "<<evhttp_request_get_response_code(req)<<endl;
    cout<<evhttp_request_get_response_code_line(req)<<endl;
    int dlen = filepath.rfind('/');
    cout<<dlen<<flush;
    string a = filepath.substr(dlen+1,sizeof(filepath)-dlen+3);
    FILE *fp = fopen(a.c_str(),"wb");
    if(!fp){
        cout<<"open file "<<filepath<<"failed!"<<endl;
    }
    char buf[1024] = {0};
    evbuffer *input =  evhttp_request_get_input_buffer(req);
    while (true)
    {
        int len = evbuffer_remove(input,buf,sizeof(buf)-1);
        if (len <= 0) break;
        buf[len] = 0;
        fwrite(buf,1,len,fp);
    }
    if(fp)
        fclose(fp);
    
    return ;
};
int main(){
    event_base *base = event_base_new();
    string http_url = "http://www.linuxidc.com/upload/2011_09/11091216073073.jpg";
    evhttp_uri *uri =  evhttp_uri_parse(http_url.c_str());
    const char *scheme = evhttp_uri_get_scheme(uri);
    if(scheme == nullptr){
        cerr<<"shceme is null"<<endl;
        return -1;
    }
    int port = evhttp_uri_get_port(uri);
    if (port < 0)
    {
        if(strcmp(scheme,"http") == 0)
            port = 80;
    }
    
    const char *host = evhttp_uri_get_host(uri);
    if(!host){
        cerr<<"host is null"<<endl;
        return -1;
    }
    const char *path = evhttp_uri_get_path(uri);
    if (!path)
    {
        path ="";
    }
    const char *query = evhttp_uri_get_query(uri);
    if (!query)
    {
        query = "/";
    }
    
    cout<<"scheme is "<<scheme<<endl;
    cout<<"host is "<<host<<endl;
    cout<<"port is "<<port<<endl;
    cout<<"path is "<<path<<endl;
    cout<<"query is"<<query<<endl;

    bufferevent *bev = bufferevent_socket_new(base,-1,BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *even = evhttp_connection_base_bufferevent_new(base,NULL,bev,host,port);
    evhttp_request *req = evhttp_request_new(http_client,bev);
    evkeyvalq* output_headers=evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers,"Host",host);
    evhttp_make_request(even,req,EVHTTP_REQ_GET,path);

    cout<<"erro"<<endl;
    if(base){
        event_base_dispatch(base);
        event_base_free(base);
    }
}