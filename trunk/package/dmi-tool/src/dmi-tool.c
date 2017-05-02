/*
 * ADI Flash Utility for VeloCloud Edge520/540
 *
 * Copyright (C) 2015, ADI Engineering
 */

/*
 * This utility uses flashrom as base to program the RCCVE flash with 
 * a board serial number. flashrom is assumed to be available in the
 * system.
 *
 * This program is compiled and verified on Fedora20 Linux. 
 * To compile using gcc
 *            gcc -luuid -o vc_flash_util vc_flash_util.c

 *
 * * Example usage:
 * Update flash image with serial number installed
 * ./vc_flash_util -u [flash image name] -s [serial number] -i [uuid] -b [board serial number] 
 *
 */ 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <uuid/uuid.h>


// #define ADI_FLASH_UTIL_DEBUG

#define VC_FLASH_SIZE 8388608  /* flash image size */
#define VC_OEM_SECTION_SIZE 256  /* OEM file size */
#define VC_SN_LENGTH 13 /* length of system serieal number string */
#define VC_BSN_LENGTH 14   /* length of board serial number */
#define VC_UUID_LENGTH  16  /* length of UUID binary 16 bytes */
#define VC_UUID_STRING_LENGTH  36  /* length of UUID string 36 bytes */
#define VC_PNAME_LENGTH 10 /* product name string 10 bytes*/
#define VC_BVERSION_LENGTH 4 /* board revision 4 bytes*/


#define VC_DMI_TOTAL_SIZE (VC_SN_LENGTH + VC_BSN_LENGTH + VC_UUID_LENGTH + VC_PNAME_LENGTH  + VC_BVERSION_LENGTH )


#define	FLASH_FILENAME	"/tmp/flash_backup.bin"
#define	OEM_FILENAME	"/tmp/adi_oem.bin"

const unsigned char version[]="01.00.00.01";


unsigned char read_flash_cmd[] = "flashrom -p internal -r " FLASH_FILENAME " &> /dev/null";
unsigned char write_flash_cmd[] = "flashrom -p internal:ich_spi_force=yes -w " FLASH_FILENAME " &> /dev/null";

unsigned char flash_filename[] = FLASH_FILENAME;
unsigned char oem_filename[] = OEM_FILENAME;


unsigned char oem_string[VC_OEM_SECTION_SIZE ] = {0};
unsigned char oem_read[VC_OEM_SECTION_SIZE ];

uuid_t uuid_bin ={0};



typedef enum {
	READ_DMI,
	PROGRAM_DMI,
	UPDATE_FLASH,
	UPDATE_FLASH_DMI,
	UNKNOWN_OP
} operations_e;
	

void check_system_retCode(int status);
int read_dmi(unsigned char *dmi); 
int create_oem_section_newDMI (unsigned char *dmi);
int create_oem_section (unsigned char *sn, unsigned  char *bsn, unsigned char *uuid);
int update_current_dmi(unsigned char *current_dmi, unsigned char *sn, unsigned char *bsn, unsigned char *uuid, unsigned char *pname, unsigned char *bversion);
int update_dmi(void);	
void oem_dmi_decode(unsigned char *dmi); 
	

void print_usage() {
	printf("vc_flash_util  -- version:%s \n Usage: vc_flash_util -r -w -s [serial_number] -i [UUID] -b [board_serial_number] -u [new_flash_image] -v -h\n", version);
	printf(" -r read the OEM serial number/uuid programmed in flash\n");
	printf(" -w write the OEM serial number/uuid into flash\n");
	printf(" -s program the system serial number into flash\n");
	printf(" -i program the UUID SERIAL NUMBER into flash\n");
	printf(" -b program the board serial number into flash\n");
	printf(" -p program product name into flash \n");
	printf(" -v program board revision into flash \n");



	printf(" -u update the flash image using the image file provided, if -s or i or b not specified, the information be retrieved from the current flash image \n");
	
	printf(" -h help\n");

	return;
}

int main(int argc, char *argv[]) {

	operations_e  my_op=UNKNOWN_OP; 
	unsigned char *s_argv=NULL, *u_argv=NULL, *b_argv=NULL, *i_argv=NULL, *p_argv=NULL, *v_argv=NULL;
	int option;
	int write_sn_flag=0, write_bsn_flag=0, write_uuid_flag=0, write_pname_flag=0, write_bversion_flag=0;
	unsigned char *sn = NULL, *uuid_string=NULL, *bsn=NULL, *product_name=NULL, *bversion=NULL;   /* pointer to serial number string */
	unsigned char *new_flash = NULL;  /* new flash image filename */
	unsigned char cmd[512]={0};
	unsigned char current_dmi[VC_DMI_TOTAL_SIZE + 1], current_sn[VC_SN_LENGTH], current_uuid[VC_UUID_LENGTH], current_bsn[VC_BSN_LENGTH];
	int i, ret;

	setlinebuf(stdout);

/* parse command line arguments */
	while ((option = getopt(argc, argv,"rwhs:u:b:i:v:p:")) != -1) {
		switch (option) {
		case 'r':	
			my_op = READ_DMI;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("r, my_op = %d\n", my_op);
			#endif

			break;

		case 'w':
			my_op =  PROGRAM_DMI;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("w, my_op = %d\n", my_op);
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

		
             	case 'b' : 
			b_argv = optarg;
			bsn = optarg;
			write_bsn_flag = 1;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("b option bsn=%s write_bsn_flag = %d\n", b_argv, write_bsn_flag);
			#endif			
                	break;

			
             	case 'i' : 
			i_argv = optarg;
			uuid_string = optarg;
			write_uuid_flag = 1;
			/* convert uuid string to binary */
			uuid_parse(uuid_string, uuid_bin);
			
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("i option uuid=%s write_uuid_flag = %d\n", i_argv, write_uuid_flag);
			#endif			
                	break;


		case 'p' : 
			p_argv = optarg;
			product_name = optarg;
			write_pname_flag = 1;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("p option product_name=%s write_pname_flag = %d\n", p_argv, write_pname_flag);
			#endif			
                	break;

		case 'v' : 
			v_argv = optarg;
			bversion = optarg;
			write_bversion_flag = 1;
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("v option bsn=%s write_bversion_flag = %d\n", v_argv, write_bversion_flag);
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

		
		case 'h' :
			printf("Help\n");
		case '?' :
                default: 
			print_usage(); 
                 	exit(-1);
		}
    	}

	if(my_op ==  UPDATE_FLASH) {
		if((write_sn_flag ==1) 
			|| (write_bsn_flag==1) 
			|| (write_uuid_flag == 1)
			|| (write_pname_flag ==1 )
			|| (write_bversion_flag == 1 )) {
			my_op =  UPDATE_FLASH_DMI;
		}
	}
	#ifdef ADI_FLASH_UTIL_DEBUG
	printf(" Debug: my_op=%d\n",my_op);
	#endif
	switch(my_op) {
		case READ_DMI:
			printf("Reading flash :\n");
			ret = system(read_flash_cmd);
			check_system_retCode(ret);
			read_dmi(current_dmi);
			
			break;

		case PROGRAM_DMI:
			printf("Programming DMI  ...\n");
			ret = system(read_flash_cmd);
			check_system_retCode(ret);
			read_dmi(current_dmi);
			/* update current dmi with the new info */
			if(write_uuid_flag == 1)
			{
				update_current_dmi(current_dmi,sn,bsn,uuid_bin,product_name,bversion);
			}
			else
			{
				update_current_dmi(current_dmi,sn,bsn,0, product_name,bversion);

			}
			create_oem_section_newDMI(current_dmi);
			update_dmi();
			ret = system(write_flash_cmd);
			check_system_retCode(ret);

			break;

		case UPDATE_FLASH:
			printf("Updating flash with current DMI info ...\n");
			/* read flash, retrieve current dmi */
			ret = system(read_flash_cmd);
			check_system_retCode(ret);
			read_dmi(current_dmi);
		
			/* create OEM section file */
			create_oem_section_newDMI(current_dmi);
			
			/* copy flash file to flash_filename */
			sprintf(cmd, "cp %s %s", new_flash, flash_filename);
			ret = system(cmd);
			check_system_retCode(ret);
			/* write the  DMI info. into the new image */
			update_dmi();

			/* write to flash */
			ret = system(write_flash_cmd);
			check_system_retCode(ret);
			break;

		case UPDATE_FLASH_DMI:
			printf("Updating flash with DMI info.  \n");
		
			ret = system(read_flash_cmd);
			check_system_retCode(ret);
			read_dmi(current_dmi);

			/* copy flash file to flash_filename */
			sprintf(cmd, "cp %s %s", new_flash, flash_filename);
			ret = system(cmd);
			check_system_retCode(ret);



			/*update serial number */
			#ifdef ADI_FLASH_UTIL_DEBUG
			printf("updating OEM section , sn=%s\n",sn);
			#endif

			update_current_dmi(current_dmi,sn,bsn,(unsigned char *)uuid_bin,product_name,bversion);
			create_oem_section_newDMI(current_dmi);

			update_dmi();
			ret = system(write_flash_cmd);
			check_system_retCode(ret);


			break;

		default:
			printf("Unkown Operation, Aborting...\n");
			exit (-1);


	}
	printf("Done. Please power cycle the board if the flash has been updated\n");

	return 0;


}

int read_dmi(unsigned char *dmi) {
	
	struct stat fileStat;
	/* unsigned unsigned char sn[VC_SN_LENGTH + 1]; */
	int i;
	FILE *fp;

	
	fp = fopen(flash_filename, "rb+");

	if(fp == NULL) {
		printf ("Failed open flash file \n");
		return -1;
	}
	
	fstat(fileno(fp),&fileStat);
	if (fileStat.st_size != VC_FLASH_SIZE  ) {
		printf("Incorrect flash image file size. \n");
		fclose(fp);
		return; 
	}

	fseek( fp, 0XF00, SEEK_SET );
	fread (oem_read, sizeof (unsigned char), VC_OEM_SECTION_SIZE,fp);

	
	for (i=0; i< (VC_DMI_TOTAL_SIZE +1); i++) {
		dmi[i] = oem_read[i];
		/* printf("read dmi[%d]=%c \n",i, dmi[i]); */
	}

	#ifdef ADI_FLASH_UTIL_DEBUG
	printf(":  %s\n", dmi);
	#endif
	printf("Current DMI:\n");
	oem_dmi_decode(dmi);

	fclose(fp);

	return 0;
}

/* create oem section file with DMI information updated */
int create_oem_section_newDMI (unsigned char *dmi) {

	FILE *fp = fopen (oem_filename, "wb");
	int i=0;

	if ((dmi == NULL) || (fp == NULL))
		return -1;

	for (i=0; i< (VC_DMI_TOTAL_SIZE +1); i++) {
		oem_string[i] = dmi[i];
		/* printf("write oem_string[%d]=%c \n",i, oem_string[i]); */
	}

	#ifdef ADI_FLASH_UTIL_DEBUG
	printf("dmi = %s oem_string=%s \n", dmi, oem_string);
	#endif

	if (fp != NULL) {
        	fwrite (oem_string, sizeof (unsigned char), VC_OEM_SECTION_SIZE, fp);
       	 	fclose (fp);
    	}
	


	return 0;
}


int create_oem_section (unsigned char *sn, unsigned char *bsn, unsigned char *uuid) {

	FILE *fp = fopen (oem_filename, "wb");

	unsigned char *pOEM =  oem_string;

	if (((sn == NULL) && (bsn == NULL ) && (uuid == NULL) ) || (fp == NULL))
		return -1;
	
	if(sn) 
	{
		strcpy(pOEM, sn);
		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("sn = %s oem_string=%s \n", sn, oem_string);
		#endif
	} 

	pOEM = pOEM + VC_SN_LENGTH;



	if(uuid)
	{
		strcpy(pOEM, uuid);
		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("uuid = %s oem_string=%s \n", sn, oem_string);
		#endif

		
	}
	pOEM = pOEM + VC_UUID_LENGTH;
	
	if(bsn) 
	{
		strcpy(pOEM, bsn);
		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("bsn = %s oem_string=%s \n", bsn, oem_string);
		#endif
	} 


	if (fp != NULL) {
        	fwrite (oem_string, sizeof (unsigned char), VC_OEM_SECTION_SIZE, fp);
       	 	fclose (fp);
    	}
	


	return 0;
}

/* update the dmi buffer with the new DMI info. */
int update_current_dmi(unsigned char *current_dmi, unsigned char *sn, unsigned char *bsn, unsigned char *uuid, unsigned char *pname, unsigned char *bversion) {

	
	int i;
	
	if ((sn == NULL) && (bsn == NULL ) && (uuid == NULL) && (pname == NULL) && (bversion == NULL))  
		return -1;
	
	printf("Updating current DMI..\n");

	if(sn) 
	{
		for (i=0; i< VC_SN_LENGTH ; i++)
		{
			current_dmi[i] = sn[i];
		}
		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("sn = %s current_dmi=%s \n", sn, current_dmi);
		#endif
	} 
	else
	{
		printf("No new system SN provided, using the current system SN\n");
	}




	if(uuid)
	{
		for (i=0; i< VC_UUID_LENGTH ; i++)
		{
			current_dmi[i+ VC_SN_LENGTH] = uuid[i];
		}

		#ifdef ADI_FLASH_UTIL_DEBUG
		unsigned char my_uuid_str[37];
		uuid_unparse(uuid,my_uuid_str);
		printf("uuid = %s current_dmi =%s \n", my_uuid_str, current_dmi);
		#endif

		
	}
	else
	{
		printf("No new UUID provided, using the current UUID.\n");
	}
		
	if(bsn) 
	{
		for (i=0; i< VC_BSN_LENGTH ; i++)
		{
			current_dmi[i+ VC_SN_LENGTH + VC_UUID_LENGTH] = bsn[i];
			
		}

		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("bsn = %s current_dmi=%s \n", bsn, current_dmi);
		#endif
	} 
	else
	{
		printf("No new Board SN provided, using the current Board SN\n");
	}

		
	if(pname) 
	{
		for (i=0; i< VC_PNAME_LENGTH ; i++)
		{
			current_dmi[i+ VC_SN_LENGTH + VC_UUID_LENGTH + VC_BSN_LENGTH] = pname[i];
			
		}

		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("pname = %s current_dmi=%s \n", pname, current_dmi);
		#endif
	} 
	else
	{
		printf("No new product name provided, using the current product name\n");
	}



	if(bversion) 
	{
		for (i=0; i< VC_BVERSION_LENGTH ; i++)
		{
			current_dmi[i+ VC_SN_LENGTH + VC_UUID_LENGTH + VC_BSN_LENGTH +VC_PNAME_LENGTH ] = bversion[i];
			
		}

		#ifdef ADI_FLASH_UTIL_DEBUG
		printf("bversion = %s current_dmi=%s \n", bversion, current_dmi);
		#endif
	} 
	else
	{
		printf("No new board version provided, using the current product name\n");
	}

		


	return 0;
}


int update_dmi(void) {

	FILE *fp, *fp_oem;

	struct stat fileStat;

		
	fp = fopen(flash_filename, "rb+");

	if(fp == NULL) {
		printf ("Failed open flash file \n");
		return -1;
	}

	fstat(fileno(fp),&fileStat);
	if (fileStat.st_size != VC_FLASH_SIZE  ) {
		printf("Incorrect flash image file size. \n");
		fclose(fp);
		return; 
	}

	/* seek fp to OEM_section */
	fseek( fp, 0XF00, SEEK_SET );


	fp_oem = fopen (oem_filename, "rb");
	
    
    	if (fp_oem != NULL) {

		/* read OEM section from the OEM file */
		fread (oem_read, sizeof (unsigned char), VC_OEM_SECTION_SIZE, fp_oem);
	  	#ifdef ADI_FLASH_UTIL_DEBUG
		printf("Updating OEM section..\n");
		#endif

        	fwrite (oem_read, sizeof (unsigned char), 256, fp);
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

/* check system() return code, bail out if there is an error */
void check_system_retCode(int status)
{
        int ret;
	if( (status == -1) || status) 
	{
		ret = WEXITSTATUS(status);
		printf("system() exit with return_code=%d status=%d, Exiting ...\n",status, ret);
		exit(-1);
	}

	return;
}

/* decode dmi information (system SN, UUID, board SN) that is also saved in oem section */
void oem_dmi_decode(unsigned char *dmi) 
{
	int i;
	unsigned char sn[VC_SN_LENGTH +1 ]={0}, bsn[VC_BSN_LENGTH +1]={0};
	unsigned char pname[VC_PNAME_LENGTH + 1]={0}, bversion[VC_BVERSION_LENGTH +1]={0};
	uuid_t uuid;
	unsigned char uuid_str[37];

	printf("decoding ...\n");
	sleep(1);
	for (i=0; i<VC_SN_LENGTH ; i++)
	{
		sn[i] = dmi[i];
	}

	for (i=0; i<VC_UUID_LENGTH ; i++)
	{
		uuid[i] = dmi[i+VC_SN_LENGTH ];
	}

	uuid_unparse_lower(uuid,uuid_str);

	for (i=0; i<VC_BSN_LENGTH ; i++)
	{
		bsn[i] = dmi[i+VC_SN_LENGTH+VC_UUID_LENGTH  ];
	}

	for (i=0; i<VC_PNAME_LENGTH ; i++)
	{
		pname[i] = dmi[i+VC_SN_LENGTH+VC_UUID_LENGTH +VC_BSN_LENGTH ];
	}

	for (i=0; i<VC_BVERSION_LENGTH +1 ; i++)
	{
		bversion[i] = dmi[i+VC_SN_LENGTH+VC_UUID_LENGTH+VC_BSN_LENGTH + VC_PNAME_LENGTH];
	}


	printf("SN=%s, UUID_str=%s, BSN=%s, pname=%s bversion=%s\n",
		sn, uuid_str, bsn, pname, bversion);
	
	return;

}
