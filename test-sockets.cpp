#include "Socket.h"

#include <iostream>

int main()
{
    Socket::Init();

    try {
        try {
            ServerSocket *server = ServerSocket::Listen(5001);
            std::cerr << "TCP: Mode 1" << std::endl;
            ClientSocket *inc = server->Accept(-1);
            ClientSocket *out = ClientSocket::Connect("localhost", 5002);

            inc->Send("Hello\n", 6);
            int len = 0;
            char rep[8];
            while(len < 8)
                len += out->Recv(rep+len, 8-len);
            rep[7] = '\0';
            std::cerr << rep << std::endl;

            delete server;
            delete inc;
            delete out;
        }
        catch(SocketCantUsePort &e)
        {
            std::cerr << "TCP: Mode 2" << std::endl;
            ServerSocket *server = ServerSocket::Listen(5002);
            ClientSocket *out = ClientSocket::Connect("localhost", 5001);
            ClientSocket *inc = server->Accept(-1);

            int len = 0;
            char req[6];
            while(len < 6)
                len += out->Recv(req+len, 6-len);
            req[5] = '\0';
            std::cerr << req << std::endl;
            inc->Send("Bonjour\n", 8);

            delete server;
            delete out;
            delete inc;
        }
    }
    catch(SocketError &e)
    {
        std::cerr << "TCP: Oops!" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cerr << "\nTCP testing finished\n\n";

    return 0;
}
