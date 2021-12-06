#ifndef __NRF_H__
#define __NRF_H__
// public NRF interface.
//  - see nrf-internal.h for a lot of helper routines.

// defines a bunch of helpers.
#include "nrf-config.h"

// per pipe config: for lab, we just have one pipe.
typedef struct nrf_pipe {
    uint64_t        rxaddr;       // prx address for the given pipe: we fix at 4bytes.
    uint8_t         msg_nbytes;     // fixed size message: 1..32 bytes

    unsigned        is_retran_p:1;  // do we retran this pipe?
    cq_t            recvq;          // received message queue.

    // total bytes we've received on this pipe.
    unsigned tot_recv_bytes;
    unsigned tot_msgs;
} nrf_pipe_t;


static inline nrf_pipe_t 
nrf_pipe_mk(uint32_t rxaddr, unsigned msg_nbytes, unsigned acked_p) {
    nrf_pipe_t p = (nrf_pipe_t) {
        .rxaddr = rxaddr,
        .msg_nbytes = msg_nbytes,
        .is_retran_p = acked_p
    };
    cq_init(&p.recvq, 1);
    return p;
}


typedef struct nrf {
    uint8_t enabled_p;  // used to sanity check you've set up the nic.

    // these are assigned right into the hardware register.
    uint8_t rx_config;  // pre-computed value we can write into NRF_CONFIG
    uint8_t tx_config;  // pre-computed value we can write into NRF_CONFIG

#if 0
    // these can be assigned right into the hardware.
    uint8_t enabled_mask:6;   // set of enabled pipes.
    uint8_t autoack_mask:6;   // whether the pipe is auto-acked.
#endif

    nrf_config_t    c;      // global NRF configuration.
    nrf_pipe_t      pipe;   // for lab, we just have a single pipe.

    // stats to keep track of stuff.
    unsigned tot_sent_msgs;
    unsigned tot_sent_bytes;
    unsigned tot_retrans;
    unsigned tot_lost;
} nrf_t;

// create a simple one channel connection with <rx_addr>.
void nrf_init_acked(nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes);
void nrf_init_noacked(nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes);

// print out the NRF configuration by reading it from the hardware.
void nrf_dump(const char *msg);

// initialize the GPIO and SPI interface.
//  - the <ce> = pin used to toggle the NRF.
void nrf_gpio_init(unsigned ce);


// send to an acknowledging pipe
int nrf_send_ack(uint32_t txaddr, void *msg, unsigned nbytes);
// send to a non-acknowledging pipe
int nrf_send_noack(uint32_t txaddr, void *msg, unsigned nbytes);

// returns -1 on timeout.
int nrf_get_data_exact_timeout(uint32_t rxaddr, void *msg, unsigned nbytes,
    unsigned usec_timeout);


// get exactly <nbytes> of data.
int nrf_get_data_exact(uint32_t rxaddr, void *msg, unsigned nbytes);

void nrf_dump_raw(const char *msg);

#endif
