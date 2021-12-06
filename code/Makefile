# makefile configuration
# define this if you need to give the device for your pi
TTYUSB = 
# can override the default bootloader
BOOTLOADER = my-install

# the string to extract for checking
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'

# set if you want the code to automatically run after building.
RUN = 0
# set if you want the code to automatically check after building.
#CHECK = 0

TEST_SRC := $(wildcard ./[0-7]-test*.c)
TEST_SRC := $(wildcard ./tests/[12]-*.c)
TEST_SRC := 2-client-pingpong.c  2-server-pingpong.c

# a list of all of your object files.
SRC :=  nrf-util.c nrf-driver.c nrf-public.c ads1115_joystick.c i2c.o
# staff-nrf-driver.c
SUPPORT_OBJS := $(SRC:.c=.o) 

# CFLAGS_EXTRA  = -Iexternal-code

# these are the binaries we give you: remove these when finished.
STAFF_OBJS = spi.o

include ~/cs49n_final_project/libpi/mk/Makefile.template
