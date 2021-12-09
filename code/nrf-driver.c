#include "nrf-internal.h"
#include "package_data.h"
#include "button.h"

// these are our configurations for tx and rx --- setup so you
// can just write them out when you switch.
//
// enable crc, enable 2 byte
#   define set_bit(x) (1<<(x))
#   define enable_crc      set_bit(3)
#   define crc_two_byte    set_bit(2)
static const uint8_t
        tx_config = enable_crc | crc_two_byte | set_bit(PWR_UP),
        rx_config = tx_config | set_bit(PRIM_RX);

// for now we assume we have one pipe.
static nrf_pipe_t *nrf_pipe_get(nrf_t *n, unsigned pipeno) {
    assert(pipeno == 1);
    return &n->pipe;
}

static int nrf_rx_has_packet(void) {
    return !nrf_rx_fifo_empty();
}

// TX_FLUSH,p51: afaik we *only* do this when a reliable 
// tx failed b/c of max attempts (or, probably good:
// on boot up to put the FIFO in a good state).
static uint8_t nrf_tx_flush(void) {
    uint8_t cmd = FLUSH_TX, res = 0;
    spi_transfer(&res, &cmd, 1);
    return res;     // status.
}
static uint8_t nrf_rx_flush(void) {
    uint8_t cmd = FLUSH_RX, res = 0;
    spi_transfer(&res, &cmd, 1);
    return res;     // status.
}

uint32_t get_addr(uint8_t reg, unsigned addr_nbytes) {
    uint64_t addr = 0;
    nrf_getn(reg, &addr, addr_nbytes);
    return addr;
}

void put_addr(uint8_t reg, uint32_t addr, unsigned addr_nbytes) {
    nrf_putn(NRF_WR_REG | reg, &addr, addr_nbytes);
}

void put_addr_chk(uint8_t reg, uint32_t addr, unsigned addr_nbytes) {
    put_addr(reg, addr, addr_nbytes);
    uint32_t assigned_addr = get_addr(reg, addr_nbytes);
    if (assigned_addr != addr) {
        printk("Assigned Addr: %x, Actual Addr: %x. Failing\n", assigned_addr, addr);
        nrf_dump("Failing Address Dump");
    }
    assert(assigned_addr == addr);
}


// put device in RX mode: we always keep it here other than during startup
// and during the brief moment needed to TX a packet.   we delay until 
// device settles into a valid state so that any subsequent operation is
// guaranteed to be done in a legal state (RX).
static void nrf_rx_mode(nrf_t *n) {
    // p 22: figure 4: document says to avoid the forbidden zone b/n states.
    //
    // 1. write CE to go to <Standby-I>: so we an transition in a legal state
    // 2. then switch the NRF_CONFIG registetr.
    // 3. write CE to go to RX
    // 4. delay til device settles.
    ce_lo(n->c.ce_pin);
    nrf_put8_chk(NRF_CONFIG, rx_config);
    ce_hi(n->c.ce_pin);
    delay_us(130);
    assert(nrf_is_rx());
}

// set nrf into tx mode.
static void nrf_tx_mode(nrf_t *n) {
    //nrf_debug("setting to tx\n");

    // p22: from figure 4, there is no edge directly from <RX> to <TX>.
    // thus, we first go to <Standby-I> (by setting CE=0) *then* to <TX>.
    // I don't think there is a timing requirement?
    //
    //  1. write CE to go to a legal state.
    //  2. write the TX config.
    //  I don't think we have to wait?
    ce_lo(n->c.ce_pin);
    nrf_put8_chk(NRF_CONFIG, tx_config);
    ce_hi(n->c.ce_pin);
    assert(nrf_is_tx());
}

// initialize the NRF.
nrf_t 
nrf_init(const nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes, unsigned acked_p) {
    nrf_t nic = {0};
    // gcc bug??
    memcpy(&nic.c, &c, sizeof c);
    nic.enabled_p = 1;
    nrf_gpio_init(c.ce_pin);
    nic.pipe = nrf_pipe_mk(rx_addr, msg_nbytes, acked_p);

    nic.tx_config = tx_config;
    nic.rx_config = rx_config;

    // the rule for setting parameters:
    //      p ... must do in power down, standby-I standby-II
    //
    //      on page ... it says we can modify when in PWR_DOWN or the standby's.
    //      to keep things simple, we put the device in PWR_DOWN
    nrf_put8_chk(NRF_CONFIG, 0);
    assert(!nrf_is_pwrup());

    // disable all pipes.
    nrf_put8_chk(NRF_EN_RXADDR, 0);

    if(!acked_p) {
        // reg1: disable retran.
        // reg2: enable pipes.
        nrf_put8_chk(NRF_EN_AA, 0);
        nrf_put8_chk(NRF_EN_RXADDR, set_bit(1));

        assert(!nrf_pipe_is_enabled(0));
        assert(nrf_pipe_is_enabled(1));
        assert(!nrf_pipe_is_acked(1));
    } else {
        // reg=1: p57
        // XXX: seems like both retran pipe and pipe0 have to be ENAA_P0 = 1 (p75)
        // BUG from before: didn't enable for PO.
        nrf_put8_chk(NRF_EN_AA, 0b11);

        // reg=2: p 57, enable pipes --- always enable pipe 0 for retran.
        nrf_put8_chk(NRF_EN_RXADDR, set_bit(0) | set_bit(1));

        unsigned rt_cnt = c.retran_attempts;
        unsigned rt_d = c.retran_delay;
        assert(bits_get(rt_d,4,7) == 0);

        // reg = 4: setup retran
        nrf_put8_chk(NRF_SETUP_RETR, 0b10011);

        // double check
        assert(nrf_pipe_is_enabled(0));
        assert(nrf_pipe_is_enabled(1));
        assert(nrf_pipe_is_acked(1));
        assert(nrf_pipe_is_acked(0));
    }

    // reg=3 setup address size
    // address size of 3 -> hard coded down below
    nrf_put8_chk(NRF_SETUP_AW, set_bit(0));

    // setup RX_PW_P1 and RX_ADDR_P1
    nrf_pipe_t *p = &nic.pipe;
    nrf_debug("setting rx addresses: %x, nbytes=%d\n", rx_addr, msg_nbytes);
    put_addr_chk(NRF_RX_ADDR_P1, rx_addr, 3);
    nrf_debug("getting rx address: %x, nbytes=%d\n", get_addr(NRF_RX_ADDR_P1, 3), msg_nbytes);
    nrf_put8_chk(NRF_RX_PW_P1, msg_nbytes);
    
    nrf_put8_chk(NRF_RX_PW_P0, 0);
    nrf_put8_chk(NRF_RX_PW_P2, 0);
    nrf_put8_chk(NRF_RX_PW_P3, 0);
    nrf_put8_chk(NRF_RX_PW_P4, 0);
    nrf_put8_chk(NRF_RX_PW_P5, 0);

    // reg=5: RF_CH: setup channel --- this is for all addresses.
    nrf_put8_chk(NRF_RF_CH, 0b1011111);

    // reg=6: RF_SETUP: setup data rate and power
    // datarate already has the right encoding.
    assert(!bits_intersect(c.dr, c.db));
    // Bandwidth = 2Mbps, Power = 0dBm
    nrf_put8_chk(NRF_RF_SETUP, set_bit(3) | set_bit(1));

    // reg=7: status.  p59
    // sanity check that it is empty and nothing else is set.

    // NOTE: if we are in the midst of an active system,
    // it's possible this node receives a message which will
    // change these values.   we might want to set the
    // rx addresses to something that won't get a message.
    // 
    // ideally we would do something where we use different 
    // addresses across reboots?   we get a bit of this benefit
    // by waiting the 100ms.

    // these are not guaranteed if a stray packet arrives.
    nrf_tx_flush();
    nrf_rx_flush();

    nrf_put8(NRF_STATUS, ~0);
    assert(nrf_get8(NRF_STATUS) == (0b111 << 1));

    assert(!nrf_tx_fifo_full());
    assert(nrf_tx_fifo_empty());
    assert(!nrf_rx_fifo_full());
    assert(nrf_rx_fifo_empty());

    assert(!nrf_has_rx_intr());
    assert(!nrf_has_tx_intr());
    assert(pipeid_empty(nrf_rx_get_pipeid()));
    assert(!nrf_rx_has_packet());

    // we skip reg=0x8 (observation)
    // we skip reg=0x9 (RPD)
    // we skip reg=0xA (P0)

    // we skip reg=0x10 (TX_ADDR): used only when sending.

    // reg=0x17: FIFO_STATUS p61

    // reg=0x1c dynamic payload (next register --- don't set the others!)
    assert(nrf_get8(NRF_DYNPD) == 0);

    // reg 0x1d: feature register.  we don't use it yet.
    nrf_put8_chk(NRF_FEATURE, 0);

    // XXX: flushing = the receiver will think we received,
    // but we then kill it.  I think your network protocol has to be
    // aware of this.  if we move these earlier are the other invariants
    // guaranteed?
    nrf_tx_flush();
    nrf_rx_flush();

    // i think w/ the nic is off, this better be true.
    assert(nrf_tx_fifo_empty());
    assert(nrf_rx_fifo_empty());

    // when you go to power up for first time, have to delay.
    // p20: go from <PowerDown> to <Standby-I>
    nrf_or8(NRF_CONFIG, PWR_UP);
    delay_ms(2);

    // now go to RX mode: invariant = we are always in RX except for the 
    // small amount of time we need to switch to TX to send a message.
    nrf_rx_mode(&nic);
    nrf_dump("after config\n");

    return nic;
}

// read packets into the circular queue.
int nrf_get_pkts(nrf_t *n) {
    // ERROR: very painful: as you probably recall from awhile back,
    // the datasheet reads like you will not receive packets if you
    // are not in RX mode.  *this appears to be correct* (but should
    // test more).    thus, if you check for packets before you put in
    // rx mode, you won't receive any.  this was causing a nasty problem
    // where nothing was getting received.   aiya.  so do not optimize
    // the check here --- always do it.  in tx if you want to empty
    // the rx queue first, you can check there before calling.  also,
    // when done with tx, you should put back in rx mode.
    //

    // always should be in rx config, aways should have ce_pin high.
    ce_hi(n->c.ce_pin);
    assert(nrf_get8(NRF_CONFIG) == n->rx_config);

    // *ERROR* if you are not in receive mode, you cannot check the rx_has_packet
    // we don't have to be in RX mode for this, right?
    if(!nrf_rx_has_packet()) 
        return 0;

    unsigned cnt = 0;

    // data sheet gives the sequence to follow to get packets.
    // p63: 
    //    1. read packet through spi.
    //    2. clear IRQ.
    //    3. read fifo status to see if more packets: 
    //       if so, repeat from (1) --- we need to do this now in case
    //       a packet arrives b/n (1) and (2)
    while(1) {
        unsigned pipen = nrf_rx_get_pipeid();
        if(pipen == NRF_PIPEID_EMPTY)
            panic("impossible: empty pipeid: %b\n", pipen);

        // we are just handling pipe = 1 now.
        assert(pipen == 1);

        nrf_pipe_t *p = nrf_pipe_get(n,pipen);
        assert(p);
        unsigned nbytes = p->msg_nbytes;
        //nrf_debug("received message on pipe: %d with nbytes: %d\n", pipen, nbytes);

#       define NRF_MAX_PKT 32
        uint8_t msg[NRF_MAX_PKT];

        // getn returns status: extract the pipeid from it and 
        // make sure they match.
        uint8_t status = nrf_getn(NRF_R_RX_PAYLOAD, &msg, nbytes);
        assert(pipeid_get(status) == pipen);

        // push the message on the circular queue.
        if(!cq_push_n(&p->recvq, msg, nbytes))
            panic("not enough space left for message on pipe=%d\n", pipen);

        p->tot_msgs++;
        p->tot_recv_bytes += nbytes;

        // clear RX interrupt: note since it is a binary value, there could be more 
        // packets on the queue so we have to check again.
        // p.59
        nrf_rx_intr_clr();

        cnt++;

        // if rx is empty, break out.
        if (!nrf_rx_has_packet()) break;
    }

    return cnt;
}

// send the packet: called by nrf_tx_send_noack and nrf_tx_send_ack
static void do_tx(nrf_t *n, const void *msg, unsigned nbytes) {
    assert(nrf_get8(NRF_CONFIG) == n->tx_config);

    // clock in a payload that has the same length as the receiver.
    // p.51
    // This assumes both devices are running from this same code
    nrf_pipe_t *p = nrf_pipe_get(n,1);
    assert(p);
    unsigned payload_bytes = p->msg_nbytes;

    // 9. pulse CE to transmit the packet (see timings)
    // p 23: minimum of 10 usec.
    while (nbytes > 0) {
        nrf_putn(NRF_W_TX_PAYLOAD, msg, payload_bytes);
        nbytes -= payload_bytes;
        if (nbytes > 0) msg += payload_bytes;
        ce_hi(n->c.ce_pin);
        delay_us(10);
        ce_lo(n->c.ce_pin);
    }
    // i don't think we need to settle?
}


// send a packet without ack.
int nrf_tx_send_noack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes) {
    assert(nrf_get8(NRF_CONFIG) == n->rx_config);
    while(nrf_get_pkts(n))
        ;
    nrf_tx_mode(n);
    assert(!nrf_pipe_is_enabled(0));
    assert(nrf_pipe_is_enabled(1));
    assert(!nrf_pipe_is_acked(1));
    assert(nrf_tx_fifo_empty());

    // set the tx address.
    unsigned addr_nbytes = n->c.addr_nbytes;
    put_addr_chk(NRF_TX_ADDR, txaddr, addr_nbytes);

    do_tx(n,msg,nbytes);

    // wait for send to succeed:
    //  1. wait until tx is empty.
    //  2. clear the tx interrupt.
    //  3. switch back to rx mode.
    while (!nrf_tx_fifo_empty())
        ;
    // clear interupt
    nrf_put8(NRF_STATUS, set_bit(5));
    nrf_rx_mode(n);

    return nbytes;
}

int check_interrupt(nrf_t *n) {
    if(nrf_has_tx_intr()) {
        assert(nrf_tx_fifo_empty());
        // clear the tx interrupt.
        nrf_tx_intr_clr();
        return 1;
        // done.
    // test this by setting retran to 1.
    } else if(nrf_has_max_rt_intr()) {
        // have to flush and clear the rt interrupt.
        nrf_tx_flush();
        nrf_tx_intr_clr();
        nrf_rt_intr_clr();
        nrf_rx_mode(n);
        printk("max rt intr!\n");
        return 1;
    }
    // XXX: if we transition to RX, do we lose all the TX setup?
    if(nrf_rx_fifo_full())
        panic("rx fifo is full!\n");
    return 0;
}



// send an acknowledged packet
unsigned nrf_tx_send_ack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes) {
    assert(nrf_get8(NRF_CONFIG) == n->rx_config);

    // drain the rx if it's not empty so that we can receive acks.
    // do this before we switch out of rx mode (once we're out of rx i think
    // we never receive any packets).   
    while(nrf_get_pkts(n))
        ;

    nrf_tx_mode(n);

    assert(!nrf_has_max_rt_intr());
    assert(!nrf_has_tx_intr());

    // not necessarily true, but is for our current setup.
    assert(nrf_pipe_is_enabled(0));
    assert(nrf_pipe_is_enabled(1));
    assert(nrf_pipe_is_acked(0));
    assert(nrf_pipe_is_acked(1));

    if(nrf_tx_fifo_full()) 
        panic("tx fifo is full? (afaik this should not happen.\n");


    // setup for ack:
    //  1. write address for retran and for tx.
    unsigned addr_nbytes = n->c.addr_nbytes;
    put_addr_chk(NRF_TX_ADDR, txaddr, addr_nbytes);
    // p. 60
    put_addr_chk(NRF_RX_ADDR_P0, txaddr, addr_nbytes);

    do_tx(n,msg,nbytes);

    // we need to spin until we have a tx success or we get an 
    // error in terms of the number of retran attempts.
    // on error: p48: need to to a tx_flush() to get the failed
    // packet out of the fifo.
    // 
    // p43: when ack successful, will get a tx interrupt (TX_DS,p59).  if 
    // ack not successful, will get a MAX_RT interrupt (MAX_RT,p59)

    // XXX: big problem: the whole time we are sending we won't be able to
    // receive any messages.  (tho the datasheet does say it switches to 
    // rx mode to get ack packets).  the longer you are here the more
    // stuff you lose.
    while(1) {
        // If we get an interrupt break
        if (check_interrupt(n)) break;
    }

    nrf_rx_mode(n);
    return nbytes;
}

