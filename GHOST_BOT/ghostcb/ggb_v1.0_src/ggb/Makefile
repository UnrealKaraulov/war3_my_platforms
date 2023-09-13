SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
LFLAGS = 
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif


OBJS = commandpacket.o config.o gameprotocol.o ggb.o gpsprotocol.o socket.o util.o
COBJS =
PROGS = ./ggb++

all: $(OBJS) $(COBJS) $(PROGS)

./ggb++: $(OBJS) $(COBJS)
	$(C++) -o ./ggb++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ggb++: $(OBJS) $(COBJS)

all: $(PROGS)


commandpacket.o: ggb.h commandpacket.h
config.o: ggb.h config.h
gameprotocol.o: ggb.h util.h gameprotocol.h
ggb.o: ggb.h util.h config.h socket.h commandpacket.h gameprotocol.h gpsprotocol.h
gpsprotocol.o: ggb.h util.h gpsprotocol.h
socket.o: ggb.h util.h socket.h
util.o: ggb.h util.h
