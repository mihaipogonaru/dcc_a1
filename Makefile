.PHONY: all clean
FALGS = -O0 -Wall -Wextra -lpthread

all: dcc_a1

dcc_a1: main.c utils.h phy.o cable.o l2.o checksum.o
	gcc $(FALGS) $^ -o $@

phy.o: phy/phy.c phy/phy.h phy/cable.h
	gcc $(FALGS) -c $< -o $@

cable.o: phy/cable.c phy/cable.h
	gcc $(FALGS) -c $< -o $@

l2.o: l2/l2.c l2/l2.h l2/checksum.h phy/phy.h
	gcc $(FALGS) -c $< -o $@

checksum.o: l2/crc16.c l2/checksum.h
	gcc $(FALGS) -c $< -o $@

clean:
	rm -rf *.o dcc_a1
