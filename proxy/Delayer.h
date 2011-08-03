#ifndef DELAYER_H
#define DELAYER_H

#include "Proxy.h"

/**
 * Delayer pseudo-proxy.
 *
 * Provides NetStreams that are delayed by a specified amout of time.
 */
class Delayer : public Proxy {

private:
    unsigned int m_iDelay;
    Proxy *m_pProxy;

public:
    /**
     * Constructor.
     *
     * @param delay Delay to introduce, in seconds.
     */
    Delayer(unsigned int delay, Proxy *proxy = NULL);
    virtual NetStream *Connect(const char *host, int port);

};

#endif
