#include <signal.h>
#include <poll.h>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
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

    char recvbuf[256] {};

    int maxfd {listenfd};
    pollfd pollfds[POLLSIZE] {};

    pollfds[0].fd = listenfd;


    /*
    [MY] conclusion:
    compared select(), here is where poll API optimize.
    if select() API want to cope wiht both Read & Write events,
    it requires user thrghout twice, while select only throughout once.
    */
    pollfds[0].events = POLLIN;

    while (true)
    {
        int nready = poll(pollfds, maxfd+1, -1);

        //distinguish type of returning event, it is from listenfd or accpetfd?  
        if(pollfds[0].revents & POLLIN)
        {   
            sockaddr_in client {};
            socklen_t len {sizeof(client)};
            int acceptfd = accept(pollfds[0].fd, (sockaddr*)&client, &len);

            maxfd = acceptfd > maxfd ? acceptfd : maxfd;
            cout << "maxfd " << maxfd << endl;
            pollfds[acceptfd].fd = acceptfd;
            pollfds[acceptfd].events = POLLIN;

            if(--nready <= 0)continue;
        }
        //the event is from accpectfd
        else
        {
            for(int nfd = listenfd+1; nfd <= maxfd; ++nfd)
            {
                cout << "for " << nfd << endl;
                if(pollfds[nfd].revents & POLLIN)
                {
                    cout << "now I serving " << nfd << endl;
                    int n = recv(nfd, recvbuf, 256, 0);
                    if(n <= 0)
                    {
                        close(nfd);
                        //set fd filed to -1 means no longer listening this fd.
                        pollfds[nfd].fd = -1;
                    }else
                    {
                        cout.write(recvbuf, n) << endl;
                        send(nfd, recvbuf, n, 0);
                    }
                    if(--nready <= 0) break;
                }
            }
        }
    }
}