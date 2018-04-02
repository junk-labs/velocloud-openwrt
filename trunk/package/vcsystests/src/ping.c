

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "CUnit/CUnit.h"


#define LOGFILE			"/tmp/ping-test.log"
#define INPUTFILE		"ping_input.txt"
#define CMDSIZE			4096
#define PINGIP			"127.0.0.1"

#define LINESIZE		4096
#define STRSIZE			128
#define ADDR_SIZE		17
#define TOKEN			"="

static FILE *fplog = NULL;
static char *pingip;

static int config_file_read()
{
	FILE *readfp = fopen(INPUTFILE, "r");
	char *string;
	char line[LINESIZE];
	char key[STRSIZE], value[STRSIZE];
	int ret = -1;

	if(readfp)
	{
		while ( fgets ( line, LINESIZE, readfp) != NULL ) /* read a line */
		{
			// Token will point to the part before the =.
			string = strtok(line, TOKEN);
			strcpy(key, string);
			// Token will point to the part after the =.
			string = strtok(NULL, TOKEN);
			strcpy(value, string);
			if(!strcmp(key, "PINGIP"))
			{
				strcpy(pingip, value);
				ret = 0;
			}
		}
		fclose ( readfp );
	}
	return ret;
}
static int pingtests_packetloss(size)
{
        int ret;
        char command[CMDSIZE];
        sprintf(command, "ping -s %d -w10 %s | awk '/packet loss/{if($7 != \"0%\") { exit 1}}'", size, pingip);
        ret = system(command);
	fprintf(fplog, "%s ==>> %s\n", command, ret == 0 ? "SUCCESS" : "FAILED");
        return ret;
}

static void test_check_ping(void)
{
        CU_ASSERT_EQUAL(pingtests_packetloss(64), 0);
        CU_ASSERT_EQUAL(pingtests_packetloss(1024), 0);
        CU_ASSERT_EQUAL(pingtests_packetloss(10 * 1024), 0);
        CU_ASSERT_EQUAL(pingtests_packetloss(65504), 0);
        CU_ASSERT_EQUAL(pingtests_packetloss(64), 0);
}


static int suite_init(void)
{

	if (!fplog) {
		fplog = fopen(LOGFILE, "w+");
	}
	pingip = (char *) malloc (ADDR_SIZE);
	if(config_file_read())
		strcpy(pingip, PINGIP);
	return 0;
}

static int suite_cleanup(void)
{
	free(pingip);
	fclose(fplog);
	return 0;
}



/*---------------------------------------------------------*/
static CU_TestInfo tests[] = {
	{"CheckPing", test_check_ping},


	CU_TEST_INFO_NULL
};

const CU_SuiteInfo suite_ping = {
	"ping", suite_init, suite_cleanup, tests,
};
