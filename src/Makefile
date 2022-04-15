CC=gcc
CFLAGS+=
LDFLAGS+= 

all: lancover
	@echo "compile done"

lancover: arpspoof.o arp.o
	$(CC) arpspoof.o arp.o -o $@ $(LDFLAGS)

clean:
	rm $(PROC) $(OBJS)

.PHONY:all clean
