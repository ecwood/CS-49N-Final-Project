#ifndef __TSOP_H__
#define __TSOP_H__

enum { ir_eps = 200 };
// we should never get this.
enum { NOISE = 0 } ;

struct readings { unsigned usec, v; };

const char *key_to_str(unsigned x);

// adapt your read_while_equal: return 0 if timeout passed, otherwise
// the number of microseconds + 1 (to prevent 0).
static int read_while_eq(int pin, int v, unsigned timeout) {
    unsigned start_time = timer_get_usec();
    while (1) {
        unsigned current_time = timer_get_usec();
        if (current_time - start_time > timeout) {
            return 0;
        }
        if (gpio_read(pin) != v) {
            break;
        }
    }
    return timer_get_usec() - start_time + 1;
}

// integer absolute value.
static int abs(int x);

// return 1 if <t_expect> is within eps of <t_actual>
static int within(unsigned t_expect, unsigned t_actual, unsigned eps);

// return 0 if e is within eps of lb, 1 if its within eps of ub
static int pick(struct readings *e, unsigned lb, unsigned ub);

// return 1 if is a skip: skip = delay of 550-/+eps
static int is_skip(struct readings *e);

// header is a delay of 9000 and then a delay of 4500
int is_header(struct readings *r, unsigned n);

// convert <r> into an integer by or'ing in 0 or 1 depending on the time.
// assert that they are seperated by skips!
unsigned convert(struct readings *r, unsigned n);

static void print_readings(struct readings *r, int n);

// read in values until we get a timeout, return the number of readings.  
static int get_readings(int in, struct readings *r, unsigned N) {
    unsigned timeout = 15000;
    int n, expect;
    for(n = expect = 0; n < N; n++) {
        // is the input pin high or low?
        r[n].v = expect;

        int usec = read_while_eq(in, expect, timeout);
        if (usec == 0) {
            return n;
        }
        r[n].usec = usec;
        expect = 1 - expect;
    }
    panic("too many readings\n");
}
// initialize the pin.
int tsop_init(int input);

// void notmain(void);

#endif
