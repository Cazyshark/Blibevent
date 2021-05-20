#include<event2/event.h>
#include<event2/listener.h>
#include<event2/http.h>
#include<event2/keyvalq_struct.h>
#include<string.h>
#include<string>
#include<event2/buffer.h>
#include<iostream>
#define WEBROOT "."
#define DEFAULTINDEX "index.html"
using namespace std;
void http_cb(evhttp_request *request,void* arg){
    cout<<"http_cb"<<endl;
    const char *uri = evhttp_request_get_uri(request);
    cout<<uri<<endl;
    string cmdtype;
    switch(evhttp_request_get_command(request)){
        case EVHTTP_REQ_GET:
            cmdtype ="GET";
            break;
        case EVHTTP_REQ_POST:
            cmdtype = "POST";
            break;
    }
    cout<<"cmdtype:"<< cmdtype << endl;
    evkeyvalq *headers = evhttp_request_get_input_headers(request);
    cout<<"=========headers=========="<<endl;
    for(evkeyval *p = headers->tqh_first;p!=nullptr;p= p->next.tqe_next){
        cout<<p->key<<":"<<p->value<<endl;
    }
    evbuffer *inbuf = evhttp_request_get_input_buffer(request);
    char buf[1024] = {0};
    cout<<"==========inputdata============"<<endl;
    while (evbuffer_get_length(inbuf))
    {
       int n = evbuffer_remove(inbuf,buf,sizeof(buf)-1);
       if(n>0){
           buf[n] = '\0';
           cout<<buf<<endl;
       }
    }
    string filepath =WEBROOT;
    filepath += uri;
    if (strcmp(uri,"/")==0)
    {
        filepath +=DEFAULTINDEX;
    }
    evkeyvalq *outhead = evhttp_request_get_output_headers(request);
    int pos = filepath.rfind('.');
    string postfix = filepath.substr(pos+1,filepath.size()-(pos+1));
    if (postfix == "jpg"||postfix == "gif"|| postfix=="png"||postfix == "jepg")
    {   
        string tmp = "image/"+postfix;
        evhttp_add_header(outhead,"Content-Type",tmp.c_str());
    }else if (postfix =="zip")
    {
        evhttp_add_header(outhead,"Content-Type","application/zip");
    }else if (postfix =="html"){
        evhttp_add_header(outhead,"Content-Type","text/html");
    }else if (postfix =="css"){
        evhttp_add_header(outhead,"Content-Type","text/css");
    }
    
    
    FILE *fp = fopen(filepath.c_str(),"rb");
    if (!fp)
    {
        evhttp_send_reply(request,HTTP_NOTFOUND,"",0);
        return;
    }
   
    evbuffer* outbuf = evhttp_request_get_output_buffer(request);
    while (true)
    {
        int len = fread(buf,1,sizeof(buf),fp);
        if(len <=0 )break;
        evbuffer_add(outbuf,buf,len);
    }
    
    
    evhttp_send_reply(request,HTTP_OK,"",outbuf);
    fclose(fp);
    
    
    
}
int main(){
    event_base *base = event_base_new();
    evhttp *evh =evhttp_new(base);
    
    if(evhttp_bind_socket(evh,"127.0.0.1",8080)!=0){
        cout<<"evhttp_bind_socket failed!"<<endl;
    }
    evhttp_set_gencb(evh,http_cb,0);
    if(base){
        event_base_dispatch(base);
        event_base_free(base);
        evhttp_free(evh);
    }
    return 0;
}