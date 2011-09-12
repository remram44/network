#include <SDL/SDL.h>
#include <iostream>
#include <sstream>

#include "GameEngine.h"
#include "ClientEngine.h"
#include "ServerEngine.h"

static void help(std::ostream &out)
{
    out << PONG_FULLNAME
            "Usage:\n"
            "pong [-h|--help]\n"
            "  Displays this help screen and exits\n"
            "pong [-t] <address> <port>\n"
            "  Client mode\n"
            "    <address>: host to connect to (IP or hostname)\n"
            "    <port>: port to connect to\n"
            "    -t: use TCP instead of UDP\n"
            "pong [-d] <port>\n"
            "  Server mode\n"
            "    <port>: port on which to listen (TCP and UDP)\n"
            "    -d: dedicated server (no local play)\n";
}

int main(int argc, char **argv)
{
    bool dedicated = false;
    bool tcp = false;
    const char *args[2] = {NULL, NULL};

    if(argc == 1)
    {
        help(std::cout);
        return 0;
    }

    while(*argv)
    {
        if(strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0)
        {
            help(std::cout);
            return 0;
        }
        else if(strcmp(*argv, "-d") == 0)
            dedicated = true;
        else if(strcmp(*argv, "-t") == 0)
            tcp = true;
        else if(args[0] == NULL)
            args[0] = *argv;
        else if(args[1] == NULL)
            args[1] = *argv;
        else
        {
            std::cerr << "Too many parameters on the commandline!\n";
            return 1;
        }
        argv++;
    }

    try {
        if(args[0] == NULL)
        {
            std::cerr << "At least one parameter is expected!\n\n";
            help(std::cerr);
            return 1;
        }
        else if(args[1] != NULL) // Client
        {
            if(dedicated)
            {
                std::cerr << "A client can't be dedicated; remove \"-d\"\n\n";
                help(std::cerr);
                return 1;
            }

            std::istringstream iss(args[1]);
            int port;
            iss >> port;
            if(iss.fail() || !iss.eof())
            {
                std::cerr << "Invalid port number\n";
                return 1;
            }

            ClientEngine *client;
            if(tcp)
                client = new ClientEngine(args[0], port, 1); // FIXME : proto?
            else
                client = new ClientEngine(args[0], port, 0);
            client->run();
        }
        else // Server
        {
            if(tcp)
            {
                std::cerr << "The server will always listen on both TCP and "
                        "UDP protocols; remove \"-t\"\n\n";
                help(std::cerr);
                return 1;
            }

            std::istringstream iss(args[0]);
            int port;
            iss >> port;
            if(iss.fail() || !iss.eof())
            {
                std::cerr << "Invalid port number\n";
                return 1;
            }

            ServerEngine *server = new ServerEngine(dedicated, port);
            server->run();
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "!!! FATAL ERROR !!!\n"
                "A fatal error occurred (uncatched exception). Details:\n"
                << e.what() << "\n";
        std::cerr << "Terminating...\n";
        return 2;
    }

    return 0;
}
