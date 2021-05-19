#include<iostream>
#include<event2/event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<unistd.h>
#include<thread>

using namespace std;
void read_file(evutil_socket_t fd,short event,void * arg){
    char buf[1024] = {0};
    int len = read(fd,buf,sizeof(buf)-1);
    if(len > 0){
        cout<<buf<<endl;
    }else{
        cout<<"."<<flush;
        this_thread::sleep_for(500ms);
    }   
}

int main(){
    event_config* conf = event_config_new();
    event_config_require_features(conf,EV_FEATURE_FDS);
    event_base *base = event_base_new_with_config(conf);
    event_config_free(conf);

    if(!base){
        cout<<"event_base_new_with_config failed!"<<endl;
        return -1;
    }
    int sock = open("/var/log/auth.log",O_RDONLY|O_NONBLOCK,0);
    if (sock<=0)
    {
        cerr<<"open /var/log/auth.log failed!"<<endl;
        return -2;
    }
    lseek(sock,0,SEEK_END);
   //event *fev = event_new(base,sock,EV_READ|EV_PERSIST,read_file,0);
    event *fev = event_new(base,sock,EV_READ|EV_ET,read_file,0);
    event_add(fev,nullptr);

    event_base_dispatch(base);
    event_base_free(base);
    return 0;
}