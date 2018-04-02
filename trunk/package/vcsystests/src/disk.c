

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <alloca.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "CUnit/CUnit.h"

#define __USE_GNU
#include <fcntl.h>

#define LOGFILE			"/tmp/disk-test.log"
#define CMDSIZE                 4096
#define TESTDEVICE		"/dev/mmcblk0p1"

#define SUCCESS			0
#define FAILURE			-1

static FILE *fplog = NULL;

static char **devices;
static int ndevices = 0;

static CU_pSuite pSuite = NULL;


int file_exists(const char * filename)
{
	FILE * file = NULL;
	int ret = FAILURE;
	if (file = fopen(filename, "r"))
	{
		ret = SUCCESS;
	}

	fclose(file);
	return ret;
}

static int format_disk(int bs)
{
	int ret;
	char command[CMDSIZE];
	if((ret = file_exists(TESTDEVICE) == SUCCESS))
	{
		sprintf(command, "dd if=/dev/null of=%s bs=%d", TESTDEVICE, bs);
		ret = system(command);
		fprintf(fplog, "%s ==>> %s\n", command, ret == 0 ? "SUCCESS" : "FAILED");

		/* To flush the pending buffers */
		sprintf(command, "sync");
		system(command);
	}
        return ret;

}


static int disk_write_read(char *dev, size_t block, size_t count)
{
	FILE *f = NULL;
	int fd = -1;
	int randfd = -1;
	char *rbuff;
	char *wbuff;
	int i;

	/* Test if we can open the device */
	fd = open(dev, O_RDWR);
	if (fd == -1) {
		fprintf(fplog, "open() failed: %s (%d)\n", strerror(errno), errno);
	}
	CU_ASSERT(fd != -1);
	f = fdopen(fd, "wb");

	if (f) {
		/* Allocate buffers on stack */
		rbuff = alloca(block);
		wbuff = alloca(block);

		/* Fill our write buffer with a pattern = 0xFF */
		randfd = open("/dev/urandom", O_RDONLY);
		if (randfd <= 0){
			perror("open urandom");
			return -1;
		}
		block = read(randfd, wbuff, block);
		//memset(wbuff, 0xFF, block);
		/* Clear out our read buffer */
		bzero(rbuff, block);


		for (i = 0; i < count; i++) {
			int nrd, nwr, isequal;


			lseek(fd, i * block, SEEK_SET); // TODO: Optimize this!!

			/* Write the buffer */
			fprintf(fplog, "[%s] writing %d bytes @ offset: %d\n", dev, block, i * block);
			nwr = write(fd, wbuff, block); //, 1, f);

			lseek(fd, i * block, SEEK_SET); // TODO: Optimize this!!
			/* Read back the buffer */
			fprintf(fplog, "[%s] Reading %d bytes @ offset: %d\n", dev, block, i * block);
			nrd = read(fd, rbuff, block); //, 1, f);
			fprintf(fplog, "[%s] Actually Reading %d bytes @ offset: %d\n", dev, nrd, i * block);

			/* Start Checks */
			CU_ASSERT_EQUAL(nwr, nrd);

			isequal = memcmp(rbuff, wbuff, block);
			CU_ASSERT(isequal == 0);
		}
	} else {
		fprintf(fplog, "Unable to open: %s\n", dev);
		return -1;
	}

	return 0;
}

static void test_check_disk(void)
{
	int i;
	for (i = 0; i < ndevices; i++) {
		CU_ASSERT_EQUAL(disk_write_read(devices[i], 1024, 256*1024), 0);
	}
	//CU_ASSERT_EQUAL(format_disk(512), 0);
	return;
}


static int suite_init(void)
{
	return 0;
}

static int suite_cleanup(void)
{
	int i;
	fclose(fplog);
	for (i = 0; i < ndevices; i++) {
		free(devices[i]);
	}
	free(devices);
	return 0;
}

int init_disk_suite(void)
{
	char name[64];
	char dev[64];
	int i = 0;
	FILE *f = NULL;

	if (!fplog)
		fplog = fopen(LOGFILE, "w+");


	pSuite = CU_add_suite("disk", suite_init, suite_cleanup);
	if (pSuite == NULL)
		return -1;

	devices = calloc(16, sizeof(char *));

	/* Find out what devices exist. if none, then bail. */
	while (1) {
		sprintf(dev, "/dev/mmcblk%dp1", i);
		f = fopen(dev, "rw+");
		if (f) {
			fclose(f);
			devices[i] = strdup(dev);
			fprintf(fplog, "Found a device @ %s\n", dev);
			sprintf(name, "IO_MMC%d", i);
			if (NULL == CU_add_test(pSuite, name, test_check_disk)) {
				return -1;
			}
		}
		else {
			break;
		}
		i++;
	}

	ndevices = i;

	if (ndevices == 0) {
		fprintf(fplog, "No devices found. Cant run any disk tests.\n");
	}

	return 0;

}

