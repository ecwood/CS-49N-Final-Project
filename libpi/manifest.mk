# name of the library we emit.
LIB = libpi.a

# set this to the path of your ttyusb device if it can't find it
# automatically
TTYUSB = 

# the string to extract for checking
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'

# set if you want the code to automatically run after building.
RUN = 1
# set if you want the code to automatically check after building.
#CHECK = 0

ifdef CS49N_STAFF
STAFF_OBJS += staff-objs/kmalloc.o
endif

include $(CS49N_2021_PATH)/libpi/mk/Makefile.lib.template

test:
	make -C  tests check

all:: ./staff-start.o

staff-start.o: ./objs/staff-start.o
	cp ./objs/staff-start.o staff-start.o

clean::
	rm -f staff-start.o
	make -C  tests clean
