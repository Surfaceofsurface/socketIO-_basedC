#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
int main()
{
    using namespace std;
    //AF:Address Famliy
    //SOCK_STREAM: TCP protocol
    //0: unknown
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    //Set server's IP & port
    sockaddr_in sock {};
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.sin_port = ntohs(11455);

    if(connect(socketfd, (sockaddr*)&sock, sizeof(sock)) < 0)
    {
        cout << "connect Error" << endl;
        close(socketfd);
        return -1;
    };

    //send data to server
    string userinput {};
    time_t t = time(0);
    char recvbuf[256] {};
    int recvnum {0};
    while(true)
    {
        cout << to_string(localtime(&t)->tm_sec)
            <<"Entering anything you want to send:" << endl;
        getline(cin, userinput);
        send(socketfd, userinput.c_str(), userinput.size(), 0);
        cout << "-------!-RECV-DATA-!----------" << endl;

        int recvnum = recv(socketfd, recvbuf, 256, 0);
        if(recvnum > 0)
        {
            cout.write(recvbuf, recvnum) << endl;
        }
    };
    close(socketfd);
    return 0;
}