// ssd-stress.c v1 Sandra Berndt
// (c) velocloud inc, 2016

#define _LARGEFILE64_SOURCE 

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <linux/fs.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>


// globals;

char *Cmd;
char *Disk = NULL;
#define POLY 0x04c11db7

// usage help;

void
Usage(void)
{
	fprintf(stderr, "usage: %s raw-disk-dev [seed] [nblks]\n", Cmd);
	exit(-1);
}

// fill page based on seed;

void
FillPage(void *page, unsigned int size, unsigned int seed)
{
	unsigned int *p = page;

	for(size >>= 2; size; size--) {
		*p++ = seed;
		if(seed & 0x80000000)
			seed = (seed << 1) ^ POLY;
		else
			seed = (seed << 1);
	}
}


// main entry;

int
main(int argc, char **argv)
{
	int check, fd, rv;
	unsigned int seed, count, mask;
	unsigned long long maxb, ull, nblks, nb, blk, badblks;
	unsigned long *sbuf;
	off64_t off;
	unsigned char wpage[4096], rpage[4096];
	time_t t0, t1, dt;

	// first arg is raw disk drive;

	Cmd = argv[0];
	check = (strstr(Cmd, "ssd-check") != NULL);
	if(argc < 2)
		Usage();
	Disk = argv[1];

	// second [optional] arg is seed, or '.' for default;

	seed = 0;
	if(argc >= 3)
		seed = strtoul(argv[2], NULL, 0);
	if( !seed)
		seed = time(NULL);

	// third [optional] arg is max # of blocks to use;

	maxb = 0;
	if(argc >= 4)
		maxb = strtoull(argv[3], NULL, 0);

	// open raw disk;

	fd = open(Disk, O_RDWR, 0);
	if(fd < 0) {
		fprintf(stderr, "%s: cannot open %s: %s\n", Cmd, Disk, strerror(errno));
		exit(1);
	}

	// get size of block device;

	rv = ioctl(fd, BLKGETSIZE64, &ull);
	if(rv < 0) {
		fprintf(stderr, "%s: cannot get size: %s\n", Cmd, strerror(errno));
		exit(2);
	}
	nblks = ull >> 12;
	if(maxb)
		maxb >>= 12;
	else
		maxb = nblks;
	printf(" %s: %llu bytes, %llu 4k-blocks, use %llu\n", Disk, ull, nblks, maxb);

	// allocate buffer for last seed per 4k-block;

	sbuf = malloc(nblks * sizeof(unsigned long));
	if( !sbuf) {
		fprintf(stderr, "%s: cannot alloc seed buffer\n", Cmd);
		exit(3);
	}

	// bitmask to AND nblks bucket;

	for(mask = 1; mask < 0xffffffff; mask = (mask << 1) | 1) {
		if(mask >= nblks)
			break;
	}

	// test loop;
	// pull a random within 0..nblks;
	// pull second random for crc32 seed of 4k data block;
	// write nblk data blocks;
	// some may not be written, others multiple times, but sequence is reproducible;
	// keep track of seed of last write;

	for(count = 0;; count++) {
		srandom(seed);
		time(&t0);
		printf(" loop %d%s, seed %08x, %s", count, check? " check" : "", seed, ctime(&t0));
		bzero(sbuf, nblks * sizeof(unsigned long));

		// write blocks in random order with random data;

		if(check)
			printf("  regenerate ...\n");
		else
			printf("  writing ...\n");
		time(&t0);
		for(nb = 0; nb < maxb; nb++) {

			// get random within 0...nblk;
			// get page seed;
			// only regenerate sequence for check;

			do {
				blk = random() & mask;	// random block address;
			} while(blk >= nblks);
			seed = random();		// random block seed;
			sbuf[blk] = seed;
			if(check)
				continue;
			FillPage(wpage, sizeof(wpage), seed);
			//printf(" blk %llu seed %u\n", blk, seed);

			// write page to disk;

			off = lseek64(fd, blk << 12, SEEK_SET);
			if(off == -1) {
				fprintf(stderr, "%s: llseek block %llu failed: %s\n", Cmd, blk, strerror(errno));
				exit(4);
			}
			rv = write(fd, wpage, sizeof(wpage));
			if(rv != sizeof(wpage)) {
				fprintf(stderr, "%s: write block %llu failed: %s\n", Cmd, blk, strerror(errno));
				exit(5);
			}
		}
		time(&t1);
		dt = t1 - t0;
		if( !check) {
			printf("  wrote in %lu secs: %f MB/s\n", dt, (float)((maxb << 12) >> 20) / (float)dt);

			// flush core data to disk;
			// same as 'sync' command;

			time(&t0);
			printf("  flushing buffers (fsync)\n");
			rv = fsync(fd);
			if(rv < 0) {
				fprintf(stderr, "%s: fsync failed: %s\n", Cmd, strerror(errno));
				exit(7);
			}

			// instruct device to flush its write buffers/cache;
			// same as 'blockdev --flushbufs' command;

			printf("  flushing disk caches (blockdev)\n");
			rv = ioctl(fd, BLKFLSBUF, 0);
			if(rv < 0) {
				fprintf(stderr, "%s: cache flush failed: %s\n", Cmd, strerror(errno));
				exit(7);
			}
			time(&t1);
			dt = t1 - t0;
			printf("  flushed in %lu secs\n", dt);
			printf("  ok to power off, use ssd-check and seed to check after reboot\n");
		}

		// read and check blocks sequentially;

		printf("  checking ...\n");
		badblks = 0;
		time(&t0);
		for(blk = 0; blk < nblks; blk++) {
			seed = sbuf[blk];
			if( !seed)
				continue;
			FillPage(wpage, sizeof(wpage), seed);

			// read page from disk;

			off = lseek64(fd, blk << 12, SEEK_SET);
			if(off == -1) {
				fprintf(stderr, "%s: llseek block %llu failed: %s\n", Cmd, blk, strerror(errno));
				exit(8);
			}
			rv = read(fd, rpage, sizeof(rpage));
			if(rv != sizeof(rpage)) {
				fprintf(stderr, "%s: read block %llu failed: %s\n", Cmd, blk, strerror(errno));
				exit(9);
			}
			if(memcmp(wpage, rpage, sizeof(rpage))) {
				printf("  block %llu mismatch\n", blk);
				badblks++;
			}
		}
		printf("  bad blocks: %llu %s\n", badblks, badblks? "- ERROR" : "");
		check = 0;

		time(&t1);
		dt = t1 - t0;
		printf("  read in %lu secs: %f MB/s\n", dt, (float)((maxb << 12) >> 20) / (float)dt);
		seed = t1;
	}

	close(fd);
	return(0);
}

