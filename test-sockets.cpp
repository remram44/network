#include "sockets/Socket.h"
#include "sockets/TCP.h"
#include "sockets/SSLSocket.h"
#include "sockets/UDP.h"

#include <iostream>

int main(int argc, char **argv)
{
    Socket::Init();

    bool do_tcp = false, do_https = false, do_ssl = false, do_udp;

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
            else if(arg == "udp")
                do_udp = true;
            else
            {
                std::cerr << "Parameter \"" << arg << "\" was not understood.\n"
                    "Recognized commands are: tcp, https, ssl, udp\n";
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
                std::cerr << "TCP: Server" << std::endl;
                TCPSocket *inc = server->Accept(-1);

                inc->Send("Hello\n", 6);
                int len = 0;
                char rep[8];
                while(len < 8)
                    len += inc->Recv(rep+len, 8-len);
                rep[7] = '\0';
                std::cerr << "Server received: \"" << rep << "\" "
                        << ((std::string("Bonjour")==rep)?"[ok]":"[fail]")
                        << std::endl;

                delete server;
                delete inc;
            }
            catch(SocketCantUsePort &e)
            {
                std::cerr << "TCP: Client" << std::endl;
                TCPSocket *out = TCPSocket::Connect("localhost", 5001);

                int len = 0;
                char req[6];
                while(len < 6)
                    len += out->Recv(req+len, 6-len);
                req[5] = '\0';
                std::cerr << "Client received: \"" << req << "\" "
                        << ((std::string("Hello")==req)?"[ok]":"[fail]")
                        << std::endl;
                out->Send("Bonjour", 8);

                delete out;

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
            std::cerr << "Connecting to https://linuxfr.org/..." << std::endl;
            SSLClient *conn = SSLClient::Connect("linuxfr.org", 443);
            const std::string req = "HEAD / HTTP/1.1\r\n"
                "Host: linuxfr.org\r\n"
                "Connection: close\r\n\r\n";
            conn->Send(req.c_str(), req.size());
            char buf[256];
            bool stop = false;
            bool begun = false;
            do {
                try {
                    int ret = conn->Recv(buf, 256);
                    buf[ret] = '\0';
                    if(!begun)
                    {
                        std::cerr << "Got response. Header:" << std::endl;
                        begun = true;
                    }
                    std::cerr << buf;
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
                serverConf.loadVerifyLocations("test-certs/ca.crt");
                SSLServer *server = SSLServer::Listen(5003, serverConf);
                std::cerr << "SSL: Server\n";
                int i;
                for(i = 0; i < 2; ++i)
                {
                    SSLClient *client;
                    if(i == 0)
                    {
                        std::cerr << "Not asking for a peer certificate"
                                << std::endl;
                        client = (SSLClient*)server->Accept(-1);
                    }
                    else /* i == 1 */
                    {
                        std::cerr << "Asking for a peer certificate, checking "
                                "against CA" << std::endl;
                        client = (SSLClient*)server->Accept(-1, true);
                    }
                    std::cerr << "Client connected. checkPeerCert()="
                            << (client->checkPeerCert()?"true":"false")
                            << " getPeerCertCN()=\"" << client->getPeerCertCN()
                            << "\"" << std::endl;
                    client->Send("Hello\n", 6);
                    delete client;
                }
                delete server;
            }
            catch(SocketCantUsePort &e)
            {
                std::cerr << "SSL: Client\n";
                int i;
                for(i = 0; i < 2; ++i)
                {
                    SSLConfig config;
                    if(i == 0)
                        std::cerr << "Not checking peer certificate, not "
                                "loading our own" << std::endl;
                    else /* i == 1 */
                    {
                        std::cerr << "Checking peer certificate against CA, "
                                "loading our own" << std::endl;
                        config.loadVerifyLocations("test-certs/ca.crt");
                        config.setCertificateFilename("test-certs/client.crt");
                        config.setPrivatekeyFilename("test-certs/client.key");
                    }
                    SSLClient *conn = SSLClient::Connect("localhost", 5003,
                            config);
                    std::cerr << "Connected to server. checkPeerCert()="
                            << (conn->checkPeerCert()?"true":"false")
                            << " getPeerCertCN()=\"" << conn->getPeerCertCN()
                            << "\"" << std::endl;
                    int len = 0;
                    char req[6];
                    while(len < 6)
                        len += conn->Recv(req+len, 6-len);
                    req[5] = '\0';
                    std::cerr << "Client received: \"" << req << "\" "
                            << ((std::string("Hello")==req)?"[ok]":"[fail]")
                            << std::endl;
                    delete conn;
                }

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

    if(do_udp)
    {
        try {
            try {
                UDPSocket *server = new UDPSocket(5001);
                std::cerr << "UDP: Server" << std::endl;

                UDPPacket packet;
                server->recvPacket(packet, true);
                if(packet[packet.size()-1] == '\n')
                {
                    packet[packet.size()-1] = '\0';
                    SockAddress4 *address = (SockAddress4*)packet.getAddress();
                    std::cerr << "Server received from "
                            << (int)address->a << "." << (int)address->b << "."
                            << (int)address->c << "." << (int)address->d << ":"
                            << (int)packet.getPort()
                            << ": \"" << packet.c_str() << "\""
                            << std::endl;
                }

                delete server;
            }
            catch(SocketCantUsePort &e)
            {
                std::cerr << "UDP: Client" << std::endl;
                UDPSocket *client = new UDPSocket();

                UDPPacket packet((const unsigned char*)"Test\n", 5);
                {
                    const SockAddress *address = Socket::Resolve("localhost");
                    packet.setAddress(address);
                    delete address;
                }
                packet.setPort(5001);
                client->sendPacket(packet);

                delete client;

                return 0;
            }
        }
        catch(SocketError &e)
        {
            std::cerr << "UDP: Oops!" << std::endl;
            std::cerr << e.what() << std::endl;
            return 1;
        }

        std::cerr << "\nUDP testing finished\n\n";
    }

    return 0;
}
