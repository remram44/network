#include "Socket.h"

#include <iostream>

int main()
{
    Socket::Init();

    try {
        ServerSocket *server = ServerSocket::Listen(5001);
        std::cerr << "Mode 1" << std::endl;
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
    catch(std::exception &e)
    {
        try {
            std::cerr << "Mode 2" << std::endl;
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
        catch(std::exception &e)
        {
            std::cerr << "Oops!" << std::endl;
            return 1;
        }
    }

    return 0;
}
