#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdint.h>
#include <errno.h>

#define IF_NAME "sw"

/* Marvell DSA Device private ioctl commands */
#define SIOCGMVREG 0x89F1
#define SIOCSMVREG 0x89F2
#define SIOCGMVMAC 0x89F3
#define SIOCSMVMAC 0x89F4
#define SIOCDMVMAC 0x89F5
#define SIOCFMVMAC 0x89F6
#define SIOCSMV1XM 0x89F7

/* Marvell DSA register types */
#define MV_REG_PORT 0
#define MV_REG_GLOBAL 1
#define MV_REG_GLOBAL2 2

/* Marvell DSA Device private ioctl data */
struct dsa_mv_reg {
	uint16_t reg_type;
	uint16_t reserved;
	uint16_t reg_num;
	uint16_t out_reg_value;
};

struct dsa_mv_atu_opdata {

	uint16_t ops_reg;
	uint16_t data_reg;
	uint8_t mac_addr[6];
	uint8_t reserved[6];
};

struct dsa_mv_atu_opdesc {
	uint8_t mac_addr[6];
	uint16_t fid;
	uint8_t entry_state;
	uint8_t flush_all_flag;
	uint8_t reserved[7];
};

struct dsa_mv_atu_reg {
	union {
		struct dsa_mv_atu_opdata out;
		struct dsa_mv_atu_opdesc in;
	} atu_op;
};


static int32_t intf_reg_read(int fd, uint16_t port_num, 
		uint16_t reg_type, uint16_t location);

static int32_t intf_reg_write(int fd, uint16_t port_num, uint16_t reg_type, 
		uint16_t location, uint16_t value);

static int mv_mac_read(int fd, uint16_t port_num, uint8_t* mac_addr);

static int mv_mac_write(int fd, uint16_t port_num, uint8_t entry_state, 
		uint8_t* mac_addr);

static int mv_mac_purge(int fd, uint16_t port_num, uint8_t* mac_addr);

static int mv_mac_dump(int fd, uint16_t port_num);

static int mv_mac_flush(int fd, uint16_t port_num, uint8_t flush_all_flag);

static void reg_rw_test(); 

static void reg_atu_rw_test(); 

static void print_usage();

static int mv_setup_8021x(int port_num);

uint8_t base_macaddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t write_mac_addr[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

uint8_t eap_mcastaddr[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};

uint8_t client_mac_addr[] = {0xB8, 0x88, 0xE3, 0xEB, 0xDA, 0x2B};

uint16_t reg_num = 0xF;
uint16_t port_num = 0;

int main(int argc, char** argv) {

	char rw;
	int fd;
	uint16_t reg_write_val;
	if (argc == 1) {
		print_usage();
		exit(0);
	}

	if (strncmp(argv[1], "test", 4) == 0) {
		reg_rw_test();
		reg_atu_rw_test();
	} else if ((strncmp(argv[1], "custom", 6) == 0) && (argc >= 5)) {
		rw = argv[2][0];
		fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd < 0) {
			perror("socket: ");
			exit(-1);
		}
		printf("Custom\n");
		if ((rw == 'r') && (argc == 4)) {
			port_num = atoi(argv[3]);
			reg_num = atoi(argv[4]);
			printf(" Port number: %d Reg number: %d\n", port_num, reg_num);
			if (intf_reg_read(fd, port_num, MV_REG_PORT, reg_num) < 0) {
				printf("Read failed.\n");
				exit(-1);
			}
		} else if ((rw == 'w') && (argc == 5)) {
			port_num = atoi(argv[3]);
			reg_num = atoi(argv[4]);
			reg_write_val = atoi(argv[5]);
			if (intf_reg_write(fd, port_num, MV_REG_PORT, reg_num, reg_write_val) < 0) {
				printf("write failed.\n");
				exit(-1);
			}
		} else if (rw == 'o') {
			printf("**** MAC write ******************\n");
			if (mv_mac_write(fd, port_num, 0xF, client_mac_addr) < 0) {

				printf("MAC write failed.\n");
			}
			printf("\n\n");
		} else if (rw == 'd') {
			printf("**** MAC dump ******************\n");
			if (mv_mac_dump(fd, port_num) < 0) {

				printf("MAC dump failed.\n");
			}
			printf("\n\n");
		}

		close(fd);
	} else if (strncmp(argv[1], "1xtest", 6) == 0) {
		if (mv_setup_8021x(0) < 0) {

			printf("DOT1X setup failed.\n\n\n");
		}
		printf("*********************************\n\n\n");
	} else {
		print_usage();
	}
	return 0;
}


static void reg_rw_test() 
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		exit(-1);
	}

	//Port register test
	if (intf_reg_read(fd, port_num, MV_REG_PORT, 0xF) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}

	if (intf_reg_write(fd, port_num, MV_REG_PORT, 0xF, 0xDEDE) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}

	if (intf_reg_read(fd, port_num, MV_REG_PORT, 0xF) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}

	// Global register test
	if (intf_reg_read(fd, port_num, MV_REG_GLOBAL, 0x1C) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}

	if (intf_reg_write(fd, port_num, MV_REG_GLOBAL, 0x1C, 0x201F) < 0) {
		printf("Write failed.\n");
		exit(-1);
	}

	if (intf_reg_read(fd, port_num, MV_REG_GLOBAL, 0x1C) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}

	//GLobal2 register test
	if (intf_reg_read(fd, port_num, MV_REG_GLOBAL2, 0x2) < 0) {
		printf("Read failed.\n");
		exit(-1);
	}
	printf("\n\n\n");
	close(fd);
}



static void reg_atu_rw_test()
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket: ");
		exit(-1);
	}

	printf("**** MAC DUMP *******************\n");
	if (mv_mac_dump(fd, port_num) < 0) {

		printf("MAC DUMP failed.\n");
	}
	printf("\n\n");

	printf("**** MAC write ******************\n");
	if (mv_mac_write(fd, port_num, 0xF, write_mac_addr) < 0) {

		printf("MAC write failed.\n");
	}
	printf("\n\n");

	printf("**** MAC DUMP *******************\n");
	if (mv_mac_dump(fd, port_num) < 0) {

		printf("MAC Dump failed.\n");
	}
	printf("\n\n");

	printf("**** MAC READ *******************\n");
	if (mv_mac_read(fd, port_num, base_macaddr) < 0) {

		printf("MAC read failed.\n");
	}
	printf("\n\n");

	printf("**** MAC PURGE *******************\n");
	if (mv_mac_purge(fd, port_num, write_mac_addr) < 0) {

		printf("MAC purge failed.\n");
	}
	printf("\n\n");

	printf("**** MAC DUMP *******************\n");
	if (mv_mac_dump(fd, port_num) < 0) {

		printf("MAC Dump failed.\n");
	}
	printf("\n\n");

	printf("**** MAC write ******************\n");
	if (mv_mac_write(fd, port_num, 0xF, write_mac_addr) < 0) {

		printf("MAC write failed.\n");
	}
	printf("\n\n");

	printf("**** MAC FLUSH *******************\n");
	if (mv_mac_flush(fd, port_num, 0) < 0) {

		printf("MAC Dump failed.\n");
	}
	printf("\n\n");

	printf("**** MAC DUMP *******************\n");
	if (mv_mac_dump(fd, port_num) < 0) {

		printf("MAC Dump failed.\n");
	}

	printf("**** MAC FLUSH *******************\n");
	if (mv_mac_flush(fd, port_num, 1) < 0) {

		printf("MAC Dump failed.\n");
	}
	printf("\n\n");

	printf("**** MAC DUMP *******************\n");
	if (mv_mac_dump(fd, port_num) < 0) {

		printf("MAC Dump failed.\n");
	}
	printf("\n\n");
	close(fd);

}

static int32_t intf_reg_read(int fd, uint16_t port_num, 
		uint16_t reg_type, uint16_t location)
{
	struct ifreq if_reg;
	struct dsa_mv_reg* msg = (struct dsa_mv_reg*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	msg->reg_type = reg_type;
	msg->reg_num = location;

	int ioctl_out = ioctl(fd, SIOCGMVREG, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCGMVREG on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl out: %u  Read value: %u  %02X\n", ioctl_out, 
			msg->out_reg_value, msg->out_reg_value);
	return msg->out_reg_value;
}

static int32_t intf_reg_write(int fd, uint16_t port_num, uint16_t reg_type, 
		uint16_t location, uint16_t value)
{
	struct ifreq if_reg;
	struct dsa_mv_reg* msg = (struct dsa_mv_reg*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	msg->reg_type = reg_type;
	msg->reg_num = location;
	msg->out_reg_value = value;

	int ioctl_out = ioctl(fd, SIOCSMVREG, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCSMVREG on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl out: %u\n", ioctl_out);
	return ioctl_out;
}

static int mv_mac_read(int fd, uint16_t port_num, uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = (struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = (struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	memcpy(in_msg->mac_addr, mac_addr, 6);
	int ioctl_out = ioctl(fd, SIOCGMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCGMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	return ioctl_out;
}

static int mv_mac_write(int fd, uint16_t port_num, uint8_t entry_state, 
		uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = (struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = (struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	memcpy(in_msg->mac_addr, mac_addr, 6);
	in_msg->entry_state = entry_state;
	int ioctl_out = ioctl(fd, SIOCSMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCSMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	return ioctl_out;
}

static int mv_mac_purge(int fd, uint16_t port_num, uint8_t* mac_addr)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = (struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = (struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	memcpy(in_msg->mac_addr, mac_addr, 6);
	int ioctl_out = ioctl(fd, SIOCDMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCDMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	return ioctl_out;
}

static int mv_mac_dump(int fd, uint16_t port_num)
{
	uint8_t t_addr[6];
	uint8_t mac_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	int ioctl_out = 0;
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = (struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = (struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);
	memcpy(in_msg->mac_addr, mac_addr, 6);

	while (1) {
		ioctl_out = ioctl(fd, SIOCGMVMAC, &if_reg);
		if (ioctl_out < 0) {
			fprintf(stderr, "SIOCGMVMAC on %s failed: %s\n", if_reg.ifr_name,
					strerror(errno));
			return -1;
		}
		printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
		printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
				out_msg->mac_addr[0],
				out_msg->mac_addr[1], out_msg->mac_addr[2],
				out_msg->mac_addr[3],
				out_msg->mac_addr[4], out_msg->mac_addr[5]);
		memcpy(t_addr, out_msg->mac_addr, 6);
		memset(out_msg, 0, sizeof(struct dsa_mv_atu_opdata));
		memcpy(in_msg->mac_addr, t_addr, 6);
		//memcpy(out_msg->mac_addr, msg->out_mac_addr, 6);
	}
	return ioctl_out;
}

static int mv_mac_flush(int fd, uint16_t port_num, uint8_t flush_all_flag)
{
	struct ifreq if_reg;
	struct dsa_mv_atu_opdesc* in_msg = (struct dsa_mv_atu_opdesc*)(&if_reg.ifr_ifru);
	struct dsa_mv_atu_opdata* out_msg = (struct dsa_mv_atu_opdata*)(&if_reg.ifr_ifru);
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);

	in_msg->flush_all_flag = flush_all_flag;
	int ioctl_out = ioctl(fd, SIOCFMVMAC, &if_reg);
	if (ioctl_out < 0) {
		fprintf(stderr, "SIOCFMVMAC on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	printf("ioctl reg out: %02x  %02X\n", out_msg->ops_reg, out_msg->data_reg);
	printf("ioctl MAC out: %0x:%0X:%0X:%0X:%0X:%0X\n", 
			out_msg->mac_addr[0],
			out_msg->mac_addr[1], out_msg->mac_addr[2],
			out_msg->mac_addr[3],
			out_msg->mac_addr[4], out_msg->mac_addr[5]);
	return ioctl_out;

}

static int mv_setup_8021x(int port_num)
{
	int32_t fd = socket(AF_INET, SOCK_DGRAM, 0);
	int32_t rc = 0;
	struct ifreq if_reg;
	memset(&if_reg, 0, sizeof(struct ifreq));
	snprintf(if_reg.ifr_name, IFNAMSIZ, IF_NAME"%d", port_num);
	if (fd < 0) {
		perror("socket: ");
		rc = -1;
		goto error_return;
	}
	rc = ioctl(fd, SIOCSMV1XM, &if_reg);
	if (rc < 0) {
		fprintf(stderr, "SIOCSMV1XM on %s failed: %s\n", if_reg.ifr_name,
				strerror(errno));
		return -1;
	}
	close(fd);
error_return:
	return rc;
}

static void print_usage()
{
	printf("Usage \n(1)  ./<pgm> test\n \
			(2)  ./<pgm> custom [r|w] <port> <reg> <new val (if 'w')>\n \
			(3)  ./<pgm> 1xtest\n");
}
