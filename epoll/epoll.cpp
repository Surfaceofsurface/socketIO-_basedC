#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>

int main()
{

    using namespace std;
    const ushort POLLSIZE {256};
    sockaddr_in serversock {};
    serversock.sin_family = AF_INET;
    serversock.sin_addr.s_addr = htonl(INADDR_ANY);
    serversock.sin_port = ntohs(11455);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if(listenfd < 0)
    {
        cout << "socket Error" << endl;
        return 0;
    }

    if(bind(listenfd, (sockaddr*)&serversock, sizeof(serversock)) < 0)
    {
        cout << "bind Error" << endl;
        return 0;
    }

    if(listen(listenfd, 10) < 0)
    {
        cout << "listen Error" << endl;
        return 0;
    }

    char recvbuf[POLLSIZE] {};
    //if the arguments of epoll_create greater than 1, then it equals to any numbers
    //which greater than 1. 
    int epollfd = epoll_create(1);
    epoll_event ev {};
    epoll_event ev_passer[POLLSIZE] {};

    ev.data.fd = listenfd; //neccesity
    ev.events = EPOLLIN;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) != 0)
    {
        //even if epoll_ctl() has got listenfd, the listenfd in ev is neccestiy. 
        cout << "epoll_add listenfd Error" << endl;
    }

    while(true)
    {   
        int nready = epoll_wait(epollfd, ev_passer, POLLSIZE, -1);
        cout << "after wait nready" << nready << endl;

        for(int n = 0; n < nready; ++n)
        {
            if(ev_passer[n].data.fd == listenfd)
            {
                sockaddr_in client {};
                socklen_t len {sizeof(client)};
                int acceptfd = accept(listenfd, (sockaddr*)&client, &len);

                ev.events = EPOLLIN;
                ev.data.fd = acceptfd;
                cout << "accept" << acceptfd << "," << ev.data.fd <<endl;
                //use epoll_ctl to add fd to give epoll to manager.
                epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, &ev);

            }else if(ev_passer[n].events & EPOLLIN)
            {
                int count = recv(ev_passer[n].data.fd, recvbuf, POLLSIZE, 0);
                cout << "recv" << ev_passer[n].data.fd <<endl;
                if(count > 0)
                {
                    cout.write(recvbuf, count) << endl;
                    send(ev_passer[n].data.fd, recvbuf, count, 0);
                }else
                {
                    ev.events = EPOLLIN;//neccesit
                    ev.data.fd = ev_passer[n].data.fd;//neccesit
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, ev_passer[n].data.fd, &ev);
                    close(ev_passer[n].data.fd);
                }
            }
        };
    }
}