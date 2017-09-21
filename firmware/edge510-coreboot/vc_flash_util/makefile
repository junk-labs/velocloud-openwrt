
APP=vc_flash_util
CC=gcc 
LIBS = -luuid

$(APP): vc_flash_util.c
	$(CC) -o $@ $< $(LIBS)

clean:
	rm -f $(APP) *.o

install:
	cp $(APP) /usr/local/bin
