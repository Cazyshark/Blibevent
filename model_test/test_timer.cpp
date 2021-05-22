#include<iostream>
#include<event2/event.h>
using namespace std;
static timeval t1 = {1,0};
void time1(int sock,short which,void *arg){
    cout<<"[time1]"<<flush;
    event *ev = (event*)arg;
    if(!evtimer_pending(ev,&t1)){
        evtimer_del(ev);
        evtimer_add(ev,&t1);
    }
}
void time3(int sock,short which,void *arg){
    cout<<"[time3]"<<flush;
}
int main(int argc,char *argv[]){
    event_base *base = event_base_new();

    cout<<"tset timer"<<endl;

    event *evl = evtimer_new(base,time1,event_self_cbarg());
    if (!evl)
    {
        cout<<"evtimer_new time1 failed!"<<endl;
        return -1;
    }
    evtimer_add(evl,&t1);

    event *ev3 = event_new(base,-1,EV_PERSIST,time3,0);
    static timeval tv_in = {3,0};
    const timeval *t3;
    t3 =  event_base_init_common_timeout(base,&tv_in);
    event_add(ev3,t3);

    event_base_dispatch(base);
    event_base_free(base);
    return 0;
}
