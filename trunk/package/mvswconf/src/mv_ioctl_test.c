#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <stdint.h>
#include <errno.h>
#include "mv_ioctl.h"
#include "mv_vlan.h"
#include "mv_dot1x.h"

uint8_t global_flag = 0, global_type = 0;
uint8_t port_reg = 0, read_oper = 0, write_oper = 0;
uint8_t dump_flag = 0, dump_type = 0;
uint8_t flush_flag = 0, flush_type = 0;
uint8_t load_flag = 0, load_type = 0;
uint16_t reg_num = 0, write_val = 0, port_num = 0;

uint8_t bc_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void print_usage() {

	printf("Usage: %s\n", 
			"Global register read  : ./<pgm> -g <type> -r <reg num>\n \
			 Global register write : ./<pgm> -g <type> -w <reg_num> -v <val>\n \
			 Port register read    : ./<pgm> -p <port num> -r <reg num>\n \
			 Port register write   : ./<pgm> -p <port_num> -w <reg_num> -v <val>\n \
			 Dump Data             : ./<pgm> -d <type ATU - 1 VTU - 2>\n \
			 Flush Data            : ./<pgm> -f <type ATU - 1 VTU - 2>\n \
			 Load ATU Data         : ./<pgm> -l <ATU broadcast load - 1> -p <port_num>\n");
}


static void parse_input(int argc, char** argv) {

	int32_t rc = 0;
	uint8_t val_flag = 0;
	//char* port_num_arg = NULL, reg_num_arg = NULL, write_val_arg = NULL;
	while ((rc = getopt(argc, argv, "g:p:r:w:v:d:f:l:")) != -1) {
		switch (rc) {
			case 'g':
				global_flag = 1;
				global_type = atoi(optarg);
				break;
			case 'p':
				port_reg = 1;
				//port_num_arg = optarg;
				port_num = atoi(optarg);
				break;
			case 'r':
				read_oper = 1;
				//reg_num_arg = optarg;
				reg_num = atoi(optarg);
				break;
			case 'w':
				write_oper = 1;
				//reg_num_arg = optarg;
				reg_num = atoi(optarg);
				break;
			case 'v':
				write_val = atoi(optarg);
				val_flag = 1;
				break;
			case 'd':
				dump_flag = 1;
				dump_type = atoi(optarg);
				break;
			case 'f':
				flush_flag = 1;
				flush_type = atoi(optarg);
				break;
			case 'l':
				load_flag = 1;
				load_type = atoi(optarg);
				break;

			case '?':
				print_usage();
				exit(0);
			default:
				print_usage();
				exit(0);
		}
	}
	if ((dump_flag == 1) && ((dump_type == 1) || (dump_type == 2)))
		return;
	if ((flush_flag == 1) && ((flush_type == 1) || (flush_type == 2)))
		return;
	if ((load_flag == 1) && ((load_type == 1)))
		return;
	if ((global_flag) && ((global_type == 1) || (global_type == 2)))
		return;
	if ((global_flag && port_reg) || (!global_flag && !port_reg)
			|| ((read_oper) && (write_oper)) ||
			((write_oper) && (val_flag == 0)) ||
			(!write_oper && !read_oper)) {
		print_usage();
		exit(0);
	}
}

int main(int argc, char** argv) {

	int32_t fd = 0, rc = 0;
	uint16_t reg_type = 0;
	parse_input(argc, argv);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		return -1;
	}
	if (dump_flag) {
		if (dump_type == 1)
			mv_mac_dump("sw0");
		else if (dump_type == 2)
			mv_vtu_dump();
		goto ret;
	}
	if (flush_flag) {
		if (flush_type == 1)
			mv_mac_flush("sw0", 1);
		else if (flush_type == 2)
			mv_vtu_flush();
		goto ret;
	}
	if ((load_flag) && (load_type == 1)) {
		char intf[10];
		snprintf(intf, 10, "sw%d", port_num);
		if (mv_mac_write(intf, 0xFF, bc_addr) < 0)
			printf("BC load failed on sw1\n");
		goto ret;
	}

	if (global_flag) {
		printf("Global register operation\n");
		reg_type = global_type;
		port_num = 0;
	} else 
		printf("Port register operation\n");


	if (read_oper) {
		printf("READ - Port : %02X Register : %02X\n", port_num, reg_num);
		rc = mv_reg_read(fd, port_num, reg_type, reg_num);
		printf("Operation READ value : %02X\n", rc);
	} else {
		printf("WRITE - Port : %02X Register : %02X Value : %02X\n", 
				port_num, reg_num, write_val);
		rc = mv_reg_write(fd, port_num, reg_type, reg_num, write_val);
		printf("Operation WRITE return : %02X\n", rc);
	}
ret:
	close(fd);
	return 0;
}

