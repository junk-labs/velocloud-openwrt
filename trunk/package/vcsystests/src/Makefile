

VCSYSTESTS_OBJ = \
	       main.o \
	       network.o \
	       disk.o \
	       ping.o \
	       network/util.o \
	       network/mdio.o


all: vcsystests



vcsystests: $(VCSYSTESTS_OBJ)
	$(CC) $(CFLAGS) $(CDEFS) $(INCLUDES) $(VCSYSTESTS_OBJ) -o $@ $(LDFLAGS) $(LDADD)

clean:
	rm -f $(VCSYSTESTS_OBJ) vcsystests

