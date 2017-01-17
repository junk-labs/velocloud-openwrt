/*
 * ADI RCCVE Flash Utility
 *
 * Copyright (C) 2015, ADI Engineering
 */

/*
 * This utility uses flashrom as base to program the RCCVE flash with 
 * a board serial number. flashrom is assumed to be available in the
 * system.
 *
 * This program is compiled and verified on Fedora20 Linux. 
 * To compile using gcc: gcc -o adi_flash_util adi_flash_util.c
 * Example usage:
 * Update flash image with serial number installed
 * ./adi_flash_util -u [flash image name] -s [serial number (10 characters)] 
 *
 */ 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// #define ADI_FLASH_UTIL_DEBUG

#define ADI_RCCVE_FLASH_SIZE 8388608  /* flash image size */
#define ADI_RCCVE_OEM_SECTION_SIZE 256  /* OEM file size */
#define ADI_RCCVE_SN_LENGTH 10 /* length of serieal number string */

#define	FLASH_FILENAME	"/tmp/flash_backup.bin"
#define	OEM_FILENAME	"/tmp/adi_oem.bin"
#define BIOS_LAYOUT "/tmp/bios_layout"

const char version[]="01.00.00.01";


char read_flash_cmd[] = "flashrom -p internal -r " FLASH_FILENAME " &> /dev/null";

static char write_full_flash_cmd[] = "flashrom -p internal:ich_spi_force=yes -w " FLASH_FILENAME " &> /dev/null";
 
static char write_4kflash_cmd[] = "flashrom -p internal:ich_spi_force=yes -l " BIOS_LAYOUT " -i 4kdescriptor -w " FLASH_FILENAME " &> /dev/null";


char *write_flash_cmd = write_full_flash_cmd ;

char flash_filename[] = FLASH_FILENAME;
char oem_filename[] = OEM_FILENAME;


char oem_string[ADI_RCCVE_OEM_SECTION_SIZE ] = {0};
char oem_read[ADI_RCCVE_OEM_SECTION_SIZE ];


typedef enum {
	READ_SN,
	PROGRAM_SN,
	UPDATE_FLASH,
	UPDATE_FLASH_SN,
	UNKNOWN_OP
} operations_e;
	
int create_bios_layout(void) ;
		

void print_usage() {
	printf("Usage: adi_flash_util -r -w -s [serial_number] -u [new_flash_image] -v -h\n");
	printf(" -r read the OEM serial number programmed in flash\n");
	printf(" -w write the OEM serial number into flash\n");
	printf(" -s serial number will program the serial number into flash\n");
	printf(" -u update the flash image using the image file provided, if -s not specified, serial number will be retrieved from the current flash image \n");
	printf(" -v print adi_flash_util version\n");
	printf(" -h help\n");

	return;
}

int main(int argc, char *argv[]) {

	operations_e  my_op=UNKNOWN_OP; 
	char *s_argv=NULL, *u_argv=NULL;
	int option;
	int write_sn_flag=0;
	char *sn = NULL;   /* pointer to serial number string */
	char *new_flash = NULL;  /* new flash image filename */
	char cmd[512]={0};
	char current_sn[ADI_RCCVE_SN_LENGTH + 1];
	int ret;


/* parse command line arguments */
	while ((option = getopt(argc, argv,"rwvs:u:")) != -1) {
		switch (option) {
		case 'r':	
			my_op = READ_SN;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("r, my_op = %d\n", my_op);
			#endif

			break;

		case 'w':
			my_op =  PROGRAM_SN;
			write_sn_flag = 1;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("r, my_op = %d\n", my_op);
			#endif
			break;
		

             	case 's' : 
			s_argv = optarg;
			sn = optarg;
			write_sn_flag = 1;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("s option sn=%s write_sn_flag = %d\n", s_argv, write_sn_flag);
			#endif			
                	break;
             	
		case 'u' :
			u_argv = optarg; 

			my_op = UPDATE_FLASH;
			new_flash = optarg;
			
			#ifdef ADI_FLASH_UTIL_DEBUG

			printf("s, my_op = %d\n", my_op);

			printf("u option flashfile=%s\n", new_flash);
			#endif

                	break;

		case 'v':
			printf("ADI_RCCVE_FLASH_UTILITY version:%s\n",version);
			exit(0);

		case 'h' :
		case '?' :
                default: 
			print_usage(); 
                 	exit(-1);
		}
    	}

	if(my_op ==  UPDATE_FLASH) {
		if(write_sn_flag ==1) {
			my_op =  UPDATE_FLASH_SN;
		}
	}

	/* create BIOS layout file */
	ret = create_bios_layout();
	if (ret != 0)
	{
		printf("Failed to create BIOS layout file, exiting ...\n");
		return -1;
	}


	#ifdef ADI_FLASH_UTIL_DEBUG
	printf(" Debug: my_op=%d\n",my_op);
	#endif
	switch(my_op) {
		case READ_SN:
			printf("Reading flash :\n");
			system(read_flash_cmd);
			read_sn(current_sn);
			printf("Serial Number Read from Flash %s\n",current_sn);
			break;

		case PROGRAM_SN:
			printf("Programming Serial Number ...\n");
			system(read_flash_cmd);
			create_oem_section(sn);
			update_sn();
			write_flash_cmd = write_4kflash_cmd;
			system(write_flash_cmd);

			break;

		case UPDATE_FLASH:
			printf("Updating flashing ...\n");
			/* read flash, retrieve current serial number */
			system(read_flash_cmd);
			read_sn(current_sn);
			printf("Serial Number Read from Flash %s\n",current_sn);

			/* create OEM section file */
			create_oem_section(current_sn);

			/* copy flash file to flash_filename */
			sprintf(cmd, "cp %s %s", new_flash, flash_filename);
			system(cmd);

			/* write the serial number into the new image */
			update_sn();

			/* write to flash */
			system(write_flash_cmd);
			break;

		case UPDATE_FLASH_SN:
			printf("Updating flash with serial number sn=%s\n",sn);
			/* copy flash file to flash_filename */
			sprintf(cmd, "cp %s %s", new_flash, flash_filename);
			system(cmd);

			/*update serial number */
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("updating OEM section , sn=%s\n",sn);
			#endif
			create_oem_section(sn);
			update_sn();
			system(write_flash_cmd);


			break;

		default:
			printf("Unkown Operation, Aborting...\n");
			/* delete bios layout file */
			system("rm "BIOS_LAYOUT"");

			exit (-1);


	}
	printf("Done. Please power cycle the board if the flash has been updated\n");

	/* delete bios layout file */
	system("rm "BIOS_LAYOUT"");

	return 0;


}

int read_sn(char *sn) {
	
	struct stat fileStat;
	/* char sn[ADI_RCCVE_SN_LENGTH + 1]; */
	int i;
	FILE *fp;

	
	fp = fopen(flash_filename, "rb+");

	if(fp == NULL) {
		printf ("Failed open flash file \n");
		return -1;
	}
	
	fstat(fileno(fp),&fileStat);
	if (fileStat.st_size != ADI_RCCVE_FLASH_SIZE  ) {
		printf("Incorrect flash image file size. \n");
		fclose(fp);
		return; 
	}

	fseek( fp, 0XF00, SEEK_SET );
	fread (oem_read, sizeof (char), ADI_RCCVE_OEM_SECTION_SIZE,fp);

	for (i=0; i< (ADI_RCCVE_SN_LENGTH +1); i++) {
		sn[i] = oem_read[i];
	}


	printf("SN:  %s\n", sn);

	fclose(fp);

	return 0;
}

int create_oem_section (char *sn) {

	FILE *fp = fopen (oem_filename, "wb");

	if ((sn == NULL) || (fp == NULL))
		return -1;

	strcpy(oem_string, sn);
	#ifdef ADI_FLASH_UTIL_DEBUG
	printf("sn = %s oem_string=%s \n", sn, oem_string);
	#endif

	if (fp != NULL) {
        	fwrite (oem_string, sizeof (char), ADI_RCCVE_OEM_SECTION_SIZE, fp);
       	 	fclose (fp);
    	}
	


	return 0;
}

int update_sn() {

	FILE *fp, *fp_oem;

	struct stat fileStat;

		
	fp = fopen(flash_filename, "rb+");

	if(fp == NULL) {
		printf ("Failed open flash file \n");
		return -1;
	}

	fstat(fileno(fp),&fileStat);
	if (fileStat.st_size != ADI_RCCVE_FLASH_SIZE  ) {
		printf("Incorrect flash image file size. \n");
		fclose(fp);
		return; 
	}

	/* seek fp to OEM_section */
	fseek( fp, 0XF00, SEEK_SET );


	fp_oem = fopen (oem_filename, "rb");
	
    
    	if (fp_oem != NULL) {

		/* read OEM section from the OEM file */
		fread (oem_read, sizeof (char), ADI_RCCVE_OEM_SECTION_SIZE, fp_oem);
	  	#ifdef ADI_FLASH_UTIL_DEBUG
		printf("Updating OEM section..\n");
		#endif

        	fwrite (oem_read, sizeof (char), 256, fp);
		fflush(fp);
			

        	fclose (fp_oem);
		fclose(fp);
    	}
	else {
		printf("Failed to open OEM file\n");
		return -1;
	}
	

	return 0;
}

int create_bios_layout(void)
{
	FILE *ptr_file;
	int x;
	ptr_file =fopen(BIOS_LAYOUT, "w");
	if (!ptr_file)
	return -1;

	fprintf(ptr_file,"# flashrom layout v2\n");

	fprintf(ptr_file,"0x00000000:0x00000fff 4kdescriptor\n");
	fprintf(ptr_file,"0x00001000:0x0007ffff restrom\n");
	fclose(ptr_file);
	return  0;
}
