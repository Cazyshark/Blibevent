#include<event2/event.h>
#include<iostream>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
#include"zlib.server.cpp"
#include"zlib.client.cpp"
#include<unistd.h>
#include<event2/listener.h>
using namespace std;

int main(){

    cout<<"test server\n";
    event_base *base = event_base_new();
    Server(base);
    Client(base);

    event_base_dispatch(base);
    event_base_free(base);
    return 0;
}
