# Do this before running make
# . ~/code.v1.5/share/makeenv/makeenv.sh local

CC = gcc
CFLAGS = -Wall -Wno-unused-variable -g -std=c99 	# pia.c use c99.

TGT = L8S2overlap
OBJ = 	L8S2overlap.o \
	hls_projection.o \
	pia.o 

	
$(TGT): $(OBJ)
	$(CC) $(CFLAGS) -o $(TGT) $(OBJ) -L$(GCTPLIB) $(GCTPLINK) 

L8S2overlap.o: L8S2overlap.c 
	$(CC) $(CFLAGS) -c L8S2overlap.c -I$(GCTPINC)

hls_projection.o: hls_projection.c
	$(CC) $(CFLAGS) -c hls_projection.c -I$(GCTPINC)

pia.o: pia.c pia.h
	$(CC) $(CFLAGS) -c pia.c

clean:
	rm -f *.o

