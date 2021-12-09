// public interface for the nrf driver.
// we do the gross thing of having a global variable.

#include "nrf-internal.h"
#include "timeout.h"

// we hard-code that there is one nrf for the whole system.  
// it's better to support multiple --- for such usage you'd
// need interrupts, so you'd have a way to associate each with
// the GPIO interrupt it was supposed to get.
nrf_t nic;

void nrf_init_noacked(nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes) {
    if(nic.enabled_p)
        panic("can only enable NRF once!\n");
    assert(c.addr_nbytes);
    nic = nrf_init(c,rx_addr,msg_nbytes,0);
}
void nrf_init_acked(nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes) {
    if(nic.enabled_p)
        panic("can only enable NRF once!\n");
    assert(c.addr_nbytes);
    nic = nrf_init(c,rx_addr,msg_nbytes,1);
    assert(nic.c.addr_nbytes);
}

// returns the number of bytes available.
// pulls data off if it's on the interface (since there can't be that much
// and we'd start dropping stuff).
int nrf_pipe_nbytes(nrf_pipe_t *p) {
    if(nrf_get_pkts(&nic)) 
        assert(cq_nelem(&p->recvq));
    return cq_nelem(&p->recvq);
}
static int 
get_data_exact_noblk(nrf_pipe_t *p, void *msg, unsigned nbytes) {
    assert(nbytes > 0);

    // explicitly, obviously return 0 if n == 0.
    unsigned n;
    if(!(n = nrf_pipe_nbytes(p)))
        return 0;
    if(n < nbytes)
        return 0;
    cq_pop_n(&p->recvq, msg, nbytes);
    return nbytes;
}

// non-blocking: if there is less than <nbytes> of data, return 0 immediately.
//    otherwise read <nbytes> of data into <msg> and return <nbytes>.
int nrf_get_data_exact_noblk(uint32_t rxaddr, void *msg, unsigned nbytes) {
    assert(rxaddr);
    nrf_pipe_t *p = &nic.pipe;
    if(rxaddr != p->rxaddr)
        panic("bad rx addr: have %x, expected %x\n", rxaddr, p->rxaddr);
    return get_data_exact_noblk(p,msg,nbytes);
}
int nrf_get_data_exact_timeout(uint32_t rxaddr, void *msg, unsigned nbytes, 
    unsigned usec_timeout) {

    assert(rxaddr);

    nrf_pipe_t *p = &nic.pipe;
    if(rxaddr != p->rxaddr)
        panic("bad rx addr: have %x, expected %x\n", rxaddr, p->rxaddr);

    timeout_t t = timeout_start();
    int n;
    while(!(n = get_data_exact_noblk(p, msg, nbytes))) {
        if(timeout_usec(&t, usec_timeout))
            return -1;
    }
    return n;
}

enum { NRF_TIMEOUT = 10};

// blocking: read exactly <nbytes> of data.
// we have it as an int in case we need to return < 0 errors.
int nrf_get_data_exact(uint32_t rxaddr, void *msg, unsigned nbytes) {
    assert(rxaddr);
    while(1) {
        unsigned usec = NRF_TIMEOUT * 1000 * 1000;
        int n = nrf_get_data_exact_timeout(rxaddr, msg, nbytes, usec);
        if(n == nbytes) 
            return n;

        if(n < 0) {
            debug("addr=%x: connection error: no traffic after %d seconds\n",
                    rxaddr,  NRF_TIMEOUT);
            nrf_dump("timeout config\n");
            return 0;
        }
        assert(n< nbytes);
    }
}

unsigned nrf_send_ack(uint32_t txaddr, void *msg, unsigned nbytes) {
    return nrf_tx_send_ack(&nic, txaddr, msg, nbytes);
}

int nrf_send_noack(uint32_t txaddr, void *msg, unsigned nbytes) {
    return nrf_tx_send_noack(&nic, txaddr, msg, nbytes);
}
