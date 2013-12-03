#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "CUnit/CUnit.h"
#include "CUnit/TestDB.h"
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"


extern CU_SuiteInfo suite_network;
extern CU_SuiteInfo suite_disk;
extern CU_SuiteInfo suite_ping;
/* --------------- ADD SUITES HERE ----------------*/

static int flag_auto = 0;
static int flag_console = 0;
static int flag_dump_test_list = 0;
char *output = NULL;
int verbose = 1;


void show_help(const char *prog)
{
	printf("Usage: %s <options>\n", prog);
	printf("  -A		Run tests in automated mode.\n");
	printf("  -C		Run tests in console mode.\n");
	printf("  -B		Run tests in basic mode. [default]\n");
	printf("  -o <name>	Output file name prefix, when run in automated mode.\n");
	printf("  -l		Dump test list, when run in automated mode.\n");

	return;
}

void process_options(int argc, char *argv[])
{
	int c;


	opterr = 0;
	while ((c = getopt(argc, argv,"AChVlo:")) != -1) {
		switch (c) {
		case 'A':
			flag_auto = 1;
			flag_console = 0;
			break;
		case 'C':
			flag_console = 1;
			flag_auto = 0;
			break;
		case 'l':
			flag_dump_test_list = 1;
			break;
		case 'o':
			output = strdup(optarg);
			break;
		case 'h':
			show_help(argv[0]);
			exit(0);
		case 'V':
			verbose++;
			if (verbose > 2) verbose = 2;
			break;
		default:
			break;

		}
	}

	if (!flag_auto && flag_dump_test_list) {
		flag_dump_test_list = 0;
	}

	if (!flag_auto && output) {
		free(output);
		output = NULL;
	}

	if (flag_dump_test_list && !output) {
		fprintf(stderr, "Error: Specify -o if -l is also specified\n");
		fflush(stderr);
		exit(-1);
	}

	return;
}


int main(int argc, char *argv[])
{
	int err = 0;

	process_options(argc, argv);


	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();


	if (init_network_suite() != 0) {
		fprintf(stderr, "Error initializing network suites\n");
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (init_disk_suite() != 0) {
		fprintf(stderr, "Error initializing disk suites\n");
		CU_cleanup_registry();
		return CU_get_error();
	}


	if (flag_auto) {

		if (output) {
			CU_set_output_filename(output);
		}
		if (flag_dump_test_list) {
			err = CU_list_tests_to_file();
			if (err != CUE_SUCCESS) {
				fprintf(stderr, "Error: dumping test list (%d): %s\n", err, CU_get_error_msg());
			}
		}
		CU_automated_run_tests();
	}
	else if (flag_console) {
		CU_console_run_tests();
	}
	else {
		CU_basic_set_mode(verbose);


		if (CUE_SUCCESS != CU_basic_run_tests()) {
			err = 1;
			goto bail;
		}
		//CU_basic_show_failures(CU_get_failure_list());
	}



bail:
	CU_cleanup_registry();

	return err;
}

