
#include "Socket.h"
#include <iostream>


int main(int argc, char **argv)
{
    try{
        Socket *sock = new Socket(8080,1024);
        //sock->accept();
        //sock->select();
        //sock->poll();
        sock->epoll();
    }catch(int &i){
        std::cout<<"client recv error:"<<i<<std::endl;
    }
    
    return 0;
}