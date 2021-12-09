#ifndef __NRF_INTERNAL_H__
#define __NRF_INTERNAL_H__
#include "rpi.h"
#include "spi.h"
#include "libc/bit-support.h"
#include "nrf.h"
// simple utility routines and shared enums.

// register numbers.  p57
enum {
    NRF_CONFIG      = 0x0,
    NRF_EN_AA       = 0x1,
    NRF_EN_RXADDR   = 0x2, // for pipe enabling
    NRF_SETUP_AW    = 0x3,
    NRF_SETUP_RETR  = 0x4,
    NRF_RF_CH       = 0x5,
    NRF_RF_SETUP    = 0x6,

    NRF_STATUS      = 0x7, // RX_P_NO has data pipe number; Write 1 to RX_DR (bit 6 to clear)
    NRF_OBSERVE_TX  = 0x8,
    NRF_RPD         = 0x9,

    NRF_RX_ADDR_P0  = 0xa, // pipe 0
    NRF_RX_ADDR_P1  = 0xb, // pipe 1
    NRF_RX_ADDR_P2  = 0xc, // pipe 2
    NRF_RX_ADDR_P3  = 0xd, // pipe 3
    NRF_RX_ADDR_P4  = 0xe, // pipe 4
    NRF_RX_ADDR_P5  = 0xf, // pipe 5

    NRF_TX_ADDR     = 0x10,

    NRF_RX_PW_P0    = 0x11,
    NRF_RX_PW_P1    = 0x12,
    NRF_RX_PW_P2    = 0x13,
    NRF_RX_PW_P3    = 0x14,
    NRF_RX_PW_P4    = 0x15,
    NRF_RX_PW_P5    = 0x16,

    NRF_FIFO_STATUS = 0x17,

    NRF_DYNPD       = 0x1c,
    NRF_FEATURE     = 0x1d,

};


// cmds: p 51
enum { 
    NRF_WR_REG              = 0b00100000,
    NRF_R_RX_PAYLOAD        = 0b01100001,
    NRF_W_TX_PAYLOAD        = 0b10100000,
    W_TX_PAYLOAD_NO_ACK     = 0b10110000,
    FLUSH_TX                = 0b11100001,
    FLUSH_RX                = 0b11100010,
    NRF_NOP                 = 0b11111111,
};

// these are simple utility routines to SPI read/write registers
// on the NRF24L01+.

// read 8 bits of data from <reg>
uint8_t nrf_get8(uint8_t reg);
// write 8 bits of data to <reg>
uint8_t nrf_put8(uint8_t reg, uint8_t v);
// write 8-bit <v> to <reg> and then check make get8(reg) = <v>.
uint8_t nrf_put8_chk(uint8_t reg, uint8_t v);

// read <nbytes> of data pointed to by <bytes> from <reg>
uint8_t nrf_getn(uint8_t reg, void *bytes, uint32_t nbytes);
// write <nbytes> of data pointed to by <bytes> to <reg>
uint8_t nrf_putn(uint8_t reg, const void *bytes, uint32_t nbytes);

// reg = reg | v
uint8_t nrf_or8(uint8_t reg, uint8_t v);

// rmw: set reg:bit=0
void nrf_bit_clr(uint8_t reg, unsigned bit);
// rmw: set reg:bit=1
void nrf_bit_set(uint8_t reg, unsigned bit);
// is reg:bit == 1?
int nrf_bit_isset(uint8_t reg, uint8_t bit_n);

# define NRF_NPIPES 6
static inline int pipe_is_legal(unsigned p) {
    return p < NRF_NPIPES;
}

/********************************************************************
 * hardware accessors.  go through and sort by register 
 * number and add page numbers.
 */

//* CONFIG=0x0, p57
static inline int nrf_is_pwrup(void) {
#   define PWR_UP   1
    return bit_isset(nrf_get8(NRF_CONFIG), PWR_UP);
}
static inline int nrf_is_rx(void) {
#   define PRIM_RX  0
    return bit_isset(nrf_get8(NRF_CONFIG), PRIM_RX);
}
static inline int nrf_is_tx(void) {
    return nrf_is_rx() == 0;
}

//* EN_AA=0x1, p57
static inline int nrf_pipe_is_acked(unsigned p) {
    assert(pipe_is_legal(p));
    return bit_get(nrf_get8(NRF_EN_AA), p) == 1;
}

//* EN_RXADDR=0x2, p57
static inline int nrf_pipe_is_enabled(unsigned p) {
    assert(pipe_is_legal(p));
    return bit_get(nrf_get8(NRF_EN_RXADDR), p) == 1;
}

//* STATUS=0x7, p59
static inline int pipeid_empty(unsigned pipeid) {
    return pipeid == 0b111;
}
static inline int pipeid_get(uint8_t status) {
    return bits_get(status, 1, 3);
}
static inline int nrf_rx_get_pipeid(void) {
#   define NRF_PIPEID_EMPTY 0b111 // p 59
    return pipeid_get(nrf_get8(NRF_STATUS));
}

// clear interrupt = write a 1 in the right position.
static inline int nrf_has_max_rt_intr(void) {
#   define MAX_RT 4
    return nrf_bit_isset(NRF_STATUS, MAX_RT);
}
static inline void nrf_rt_intr_clr(void) {
    nrf_put8(NRF_STATUS, 1 << MAX_RT);
}

static inline int nrf_has_tx_intr(void) {
#   define TX_DS 5
    return nrf_bit_isset(NRF_STATUS, TX_DS);
}
static inline void nrf_tx_intr_clr(void) {
    nrf_put8(NRF_STATUS, 1 << TX_DS);
}

static inline int nrf_has_rx_intr(void) {
#   define RX_DR 6
    return nrf_bit_isset(NRF_STATUS, RX_DR);
}
static inline void nrf_rx_intr_clr(void) {
    nrf_put8(NRF_STATUS, 1 << RX_DR);
}



//* FIFO_STATUS=0x17, p61

// is rx queue empty?
static inline int nrf_rx_fifo_empty(void) {
    return nrf_bit_isset(NRF_FIFO_STATUS, 0);
}
static inline int nrf_rx_fifo_full(void) {
    return nrf_bit_isset(NRF_FIFO_STATUS, 1);
}
static inline int nrf_tx_fifo_empty(void) {
    return nrf_bit_isset(NRF_FIFO_STATUS, 4);
}
static inline int nrf_tx_fifo_full(void) {
    return nrf_bit_isset(NRF_FIFO_STATUS, 5);
}

/****************************************************************
 * more complicated helpers.
 */

static inline uint32_t
nrf_get_addr(uint8_t reg, unsigned nbytes) {
    uint32_t x = 0;
    assert(nbytes == nrf_default_addr_nbytes);
    nrf_getn(reg, &x, nbytes);
    return x;
}

static inline void
nrf_set_addr(uint8_t reg, uint64_t addr, unsigned nbytes) {
    assert(nbytes == nrf_default_addr_nbytes);
    addr &= 0xffffff;
    // works b/c is little endien --- what is the rule for NRF_WR_REG?
    nrf_putn(reg | NRF_WR_REG, &addr, nbytes);
    // read back in and make sure it is out there.
    assert(nrf_get_addr(reg, nbytes) == addr);
}


/****************************************************************
 * other helpers.
 */
// dunno if we even need this.
static inline void ce_hi(uint8_t ce) { assert(ce==20); gpio_write(ce, 1); }
static inline void ce_lo(uint8_t ce) { assert(ce==20); gpio_write(ce, 0); }

// setup gpio pin and init SPI.  [client can do this themselves]
//
// probably:
//  1. should return a structure that // describes all the state. 
//       somewhat needed if we are going to have multiple of them.  
//  2. the pi supports two devices, so should let the user specify if 
//     its SPI device 0 or device 1 (pi suppor
// 
extern unsigned nrf_ce_pin;
void nrf_gpio_init(unsigned ce_pin);

int nrf_initial_state_check(unsigned fatal_p);
void nrf_initial_state_set(void);

extern unsigned nrf_verbose_p;
#define nrf_debug(args...) do { if(nrf_verbose_p) output(args); } while(0)
#define nrf_output(args...) do { output("NRF:"); output(args); } while(0)

//#define NDEBUG
#ifdef NDEBUG
#   define nrf_assert(bool) do { } while(0)
#else
#   define nrf_assert(bool) do {                    \
        if((bool) == 0) {                       \
            nrf_dump("dying with a panic\n");   \
            panic("%s", #bool);                 \
        }                                       \
    } while(0)
#endif


nrf_t  
nrf_init(const nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes, unsigned acked_p);
int nrf_get_pkts(nrf_t *n);
uint16_t nrf_tx_send_ack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes, unsigned red_pin, unsigned green_pin, unsigned blue_pin, unsigned on_off_pin);

int nrf_tx_send_noack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes) ;

nrf_t  
staff_nrf_init(const nrf_config_t c, uint32_t rx_addr, unsigned msg_nbytes, unsigned acked_p);
int staff_nrf_get_pkts(nrf_t *n);
int staff_nrf_tx_send_ack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes);

int staff_nrf_tx_send_noack(nrf_t *n, uint32_t txaddr, const void *msg, unsigned nbytes) ;


#endif
