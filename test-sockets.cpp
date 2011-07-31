#include "sockets/Socket.h"
#include "sockets/SSLSocket.h"

#include <iostream>

int main(int argc, char **argv)
{
    Socket::Init();

    bool do_tcp = false, do_https = false, do_ssl = false;

    if(argc > 1)
    {
        argv++;
        while(*argv)
        {
            std::string arg = *argv;
            if(arg == "tcp")
                do_tcp = true;
            else if(arg == "https")
                do_https = true;
            else if(arg == "ssl")
                do_ssl = true;
            else
            {
                std::cerr << "Parameter \"" << arg << "\" was not understood.\n"
                    "Recognized commands are: tcp, https, ssl\n";
                return 1;
            }
            argv++;
        }
    }
    else
        do_tcp = true;

    if(do_tcp)
    {
        try {
            try {
                TCPServer *server = TCPServer::Listen(5001);
                std::cerr << "TCP: Mode 1" << std::endl;
                TCPSocket *inc = server->Accept(-1);
                TCPSocket *out = TCPSocket::Connect("localhost", 5002);

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
                TCPServer *server = TCPServer::Listen(5002);
                TCPSocket *out = TCPSocket::Connect("localhost", 5001);
                TCPSocket *inc = server->Accept(-1);

                int len = 0;
                char req[6];
                while(len < 6)
                    len += out->Recv(req+len, 6-len);
                req[5] = '\0';
                std::cerr << req << std::endl;
                inc->Send("Bonjour", 8);

                delete server;
                delete out;
                delete inc;

                return 0;
            }
        }
        catch(SocketError &e)
        {
            std::cerr << "TCP: Oops!" << std::endl;
            std::cerr << e.what() << std::endl;
            return 1;
        }

        std::cerr << "\nTCP testing finished\n\n";
    }

    if(do_https)
    {
        SSLSocket::Init();

        try {
            SSLClient *conn = SSLClient::Connect("linuxfr.org", 443);
            const std::string req = "HEAD / HTTP/1.1\r\n"
                "Host: linuxfr.org\r\n\r\n";
            conn->Send(req.c_str(), req.size());
            char buf[256];
            bool stop = false;
            do {
                try {
                    int ret = conn->Recv(buf, 256);
                    buf[ret] = '\0';
                    std::cout << buf;
                }
                catch(SocketConnectionClosed &e)
                {
                    stop = true;
                }
            }
            while(!stop);
            delete conn;
        }
        catch(SocketError &e)
        {
            std::cerr << "HTTPS: Oops!" << std::endl;
            std::cerr << e.what() << std::endl;
            return 2;
        }

        std::cerr << "\nHTTPS testing finished\n";
    }

    if(do_ssl)
    {
        SSLSocket::Init();

        try {
            try {
                SSLConfig serverConf;
                serverConf.setCertificateFilename("test-certs/server.crt");
                serverConf.setPrivatekeyFilename("test-certs/server.key");
                SSLServer *server = SSLServer::Listen(5003, serverConf);
                std::cerr << "SSL: Server\n";
                SSLClient *client = (SSLClient*)server->Accept(-1);
                client->Send("Hello\n", 6);
                delete client;
                delete server;
            }
            catch(SocketCantUsePort &e)
            {
                std::cerr << "SSL: Client\n";
                SSLClient *conn = SSLClient::Connect("localhost", 5003);
                int len = 0;
                char req[6];
                while(len < 6)
                    len += conn->Recv(req+len, 6-len);
                req[5] = '\0';
                std::cerr << req << std::endl;
                delete conn;

                return 0;
            }
        }
        catch(SocketError &e)
        {
            std::cerr << "SSL: Oops!" << std::endl;
            std::cerr << e.what() << std::endl;
            return 3;
        }

        std::cerr << "\nSSL testing finished\n";
    }

    return 0;
}
