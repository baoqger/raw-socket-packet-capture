SNIFFER = sniffer

SRC = raw_socket.c
OBJ = $(SRC:.c=.o)

CC = cc
RM = rm

CFLAGS = -g
LIBS = -lpcap
LDFLAGS =


.PHONY: clean

all: $(SNIFFER)

debug: CFLAGS += -ggdb -O0
debug: $(SNIFFER)

$(SNIFFER): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(LIBS)

.c.o: 
	$(CC) -c $< $(CFLAGS)

clean:
	$(RM) ./$(OBJ) ./$(SNIFFER)