// all the definitions we use the in the test code --- easier to have in a header so that
// it can be shared b/n client and server code by updating a single place.
#define NTEST 10
#define DELAY_MS 1
#define DELAY_USEC 1

// should effectively make reliable.
#define RETRAN_ATTEMPTS 15
#define RETRAN_DELAY 500

// make these different from the defaults.
// 0xc2c2c2c2c2
// 0xe7e7e7e7e7;
#if 0
enum {
    server_addr = 0xd7d7d7d7,
    client_addr = 0xa2a2a2a2
};
#else
enum {
    server_addr = 0xd7d7d7,
    client_addr = 0xa2a2a2
};

#endif

#define test_output(iter, msg, args...)  \
    do { if((iter) % 100 == 0) output(msg, ##args); } while(0)
