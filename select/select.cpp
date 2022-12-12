#include <cstdlib>
#include <signal.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
int main()
{
    using namespace std;
    uint listenfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sock {};
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.sin_port = ntohs(11455);

    if (bind(listenfd, (sockaddr*)&sock, sizeof(sock)) < 0)
    {
        cout << "bind Error" << endl;
        return -1;
    }

    if (listen(listenfd, 10) < 0)
    {
        cout << "listen Error" << endl;
        return -1;
    }

    sockaddr_in client {};
    socklen_t len {sizeof(client)};
    fd_set rselect, rfds {};
    uint acceptfd {};
    uint maxfd { listenfd };
    FD_ZERO(&rselect);
    FD_ZERO(&rfds);
    FD_SET(listenfd, &rfds);
    while (true)
    {
        rselect = rfds;

        //if arg5 is setted to null, select will block.
        // [!important] fd_set in select() call has multiple meaning,
        // firstly, indicates OS which fd needs to be listened (if fdbit is
        // setted to 1)
        // secondly, it gives the result of select() call,which indicates
        // user which events have be triggered.
        // so I suppose all fdbits will be set to 0 after OS has judged which
        // fd needed to selected. 

        //the multiple meaning in select call() is a design fault,
        //so OS develop the poll() APIs.
        int nready = select(maxfd+1, &rselect, nullptr, nullptr, nullptr);

        //see if coming event is listenfd, if true, accept it.
        if(FD_ISSET(listenfd, &rselect))
        {   
            //accept request from client
            acceptfd = accept(listenfd, (sockaddr*)&client, &len);
            cout << "accept fd is:" << acceptfd << endl;
            //set acceptfd into rfds
            FD_SET(acceptfd, &rfds);
            if(acceptfd > maxfd) maxfd = acceptfd;
            //if --nready==0 , and it can only be listenfd,
            //so we can skip the for-loop below here.
            if(--nready == 0) 
            {
                continue;
            };
        }

        //there is no need to judge fds which range before lisntenfd. 
        for (int i= listenfd+1; i<= maxfd; ++i)
        {
            if(FD_ISSET(i, &rselect))
            {
                char recvbuf[256] {};
                int n = recv(i, recvbuf, 256, 0);
                cout.write(recvbuf, n) << endl;
                
                if(n > 0)
                {   
                    send(i, recvbuf, n, 0);
                    // argument3 only means steal n bytes(this buffer contains in 
                    // OS keneal) from socket(i),which means if client sends 1KB
                    // one time,server only recv 256 bytes,the next time recv() 
                    // call will continuetly steal 256 bytes starting 256B position.
                }else
                {
                    FD_CLR(i, &rfds);
                    close(i);
                };
                if(--nready == 0)
                {
                    break;
                };
            }
        }
    }
    close(listenfd);
    return 0;
}