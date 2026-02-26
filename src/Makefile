CC = gcc
CFLAGS = -Wall -Wextra -ggdb -O0 -I../utils
LDLIBS = -lpthread

SRCS = ring_buffer.c producer.c consumer.c packet.c ../utils/log/log.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: firewall serial

firewall: $(OBJS) firewall.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

serial: $(OBJS) serial.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) firewall.o serial.o firewall serial