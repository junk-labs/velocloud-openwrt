MVSWCONF_OBJ = \
		vlanconfig.o \
		vlan_config_read.o \
		vlan_marvell_write.o

LIBMARVELL_OBJ = \
		mv_dot1x.o \
		mv_ioctl.o \
		mv_vlan.o

all: mvswconf



mvswconf: $(MVSWCONF_OBJ) $(LIBMARVELL_OBJ)
	$(CC) $(CFLAGS) $(CDEFS) $(INCLUDES) $(MVSWCONF_OBJ) $(LIBMARVELL_OBJ) -o $@ $(LDFLAGS) $(LDADD)

clean:
	rm -f $(MVSWCONF_OBJ) $(LIBMARVELL_OBJ) mvswconf

