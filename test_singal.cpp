#include<iostream>
#include<event2/event.h>
#include<signal.h>
using namespace std;
static void Ctrl_C(int sock,short which,void *arg){
    cout<<"Ctrl_C"<<endl;
}
static void kill(int sock,short which,void *arg){
    cout<<"Ctrl_D"<<endl;
    event  *ev = (event*) arg;
    if (!evsignal_pending(ev,NULL))
    {
        event_del(ev);
        event_add(ev,NULL);
    }
    
}
int main(int argc,char*argv[]){
    event_base *base = event_base_new();
    event *csig = evsignal_new(base,SIGINT,Ctrl_C,base);
    if (!csig)
    {
        cerr<<"SIGINT evsignal_new failed!"<<endl;
        return -1;
    }

    if(event_add(csig,0)!=0){
        cerr<<"SIGINT event_add failed!"<<endl;
        return -1;
    }
    event *ksig = event_new(base,SIGTERM,EV_SIGNAL,kill,event_self_cbarg());

    event_base_dispatch(base);
    event_free(csig);
    event_base_free(base);

    return 0;
}