#include "vlanconfig.h"
#if 0
#include "common.h"

#define DBG_LEVEL 10
#endif

#define USE_TEST_CONFIG 0

char const* prog_name = "vlanconfig";

void print_usage(char *argv)
{
        printf("Usage: ./<%s>\n", argv);
}

static int vlan_log_conf()
{
#if 0
        char dbg_level[DBG_LEVEL];
	char logfile[50];
	int err;

        if (prepare_env() < 0) {
                printf("prepare_env failed.\n");
                return -1;
        }
        /* TODO: logger init - Load level from config file. */
	sprintf(logfile, "/var/log/%s.log", prog_name);
	err = logger_init(DEFAULT, logfile, DEFAULT_MAX_LOGFILE_SIZE, 
			DEFAULT_MAX_OLD_LOGFILES,
			DEFAULT_MEMLOG_SIZE, 
			LOG_ENABLE);
	if(0 != err) {
		fprintf(stderr, "Failed to allocate memory for log_ctx\n");
		exit(-1);
	}
        strncpy(dbg_level, "DEBUG", 10);
        set_dbg_level(dbg_level);
#else
	openlog(prog_name, LOG_CONS|LOG_PID, LOG_USER);
#endif
        return 0;
}


static void gen_test_config() {
	
	int i, j;
	char portmode[50];

	strcpy(portmode, "trunk");
	for (i = 0; i < LAN_TOT_PORTS; i++) {

		int *vlan_ids = (int*)malloc(VLAN_MAX * sizeof(int));
		for (j = 0; j < VLAN_MAX; j++) 
			vlan_ids[j] = j + 1;
		setVlanPorts(i, vlan_ids, VLAN_MAX, portmode, NULL, 1); 
	}
}


int main(int argc, char** argv)
{
        int ret;

        if (argc > 1) {
                print_usage(*argv);
                exit(0);
        }
	
        if (vlan_log_conf() != 0) {
                printf("Logger init failed.\n");
                exit(0);
        }
#if USE_TEST_CONFIG
	gen_test_config();
#else
        ret = parse_edge_config();
        if(ret != VLAN_SUCCESS)
        {
                DBG_LOG(SEVERE, VLAN, "%s config file parse failed\n", EDGE_CFG_FILE);
                goto cleanup;
        }
#endif

        ret = vlan_tbl_build();
        if(ret != VLAN_SUCCESS)
        {
                goto cleanup;
        }

        /* cleanup section */
cleanup:
        //vlan_tbl_free();
        return ret;
}


