/***********************************************************************************************************************************************************
*  \file       	bitloader.cpp
*  \details    	Load the bitstream to the Programable Logic through the Xilinx FPGA Manager Driver
*  \author     	Ruben Rodriguez Alvarez
*  \brief: 		For use in PYNQ-Z2 and Zynq Ultrascale+ (ZCU104)
***********************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "bitloader.hpp"

// sys/devices/platform/firmware:zynqmp-firmware/firmware:zynqmp-firmware:pcap/fpga_manager/fpga0/firmware

int BitLoader::loadBitstream(char * binfile){
	char command[200];

	/* Set flag for total bitstream */
	int fd_flags, fd_irmware;
	int err;
	char write_buf[1024];
	fd_flags = open("/sys/class/fpga_manager/fpga0/flags", O_RDWR);
	if (fd_flags < 0){
		printf("Cannot open flags file\n");
		return -1;
	}
	sprintf(write_buf, "0");
	err = write(fd_flags, write_buf, 1);
	if(err < 0){
		printf("Could not write flag\n");
		close(fd_flags);
		return -1;
	}
	close(fd_flags);
	
	/* Make directory for bitstreams if not there */
	err = system ("sudo mkdir -p /lib/firmware");
	if(err == -1){
		printf("Error in system function 2\n");
		return -1;
	}
	sprintf(command, "sudo cp %s /lib/firmware/",binfile);
	err = system (command);
	if(err == -1){
		printf("Error in system function 3\n");
		return -1;
	}
	
	/* Send binary form bitstream file to the driver */
	fd_irmware = open("/sys/class/fpga_manager/fpga0/firmware", O_WRONLY);
	if (fd_irmware < 0){
		printf("Cannot open firmware file\n");
		return -1;
	}
	sprintf(write_buf, "%s",binfile);
	err = write(fd_irmware, write_buf, strlen(write_buf)+1);
	if(err < 0){
		printf("Could not write bitstream\n");
		close(fd_irmware);
		return -1;
	}
	close(fd_irmware);

	
	return 0;
}

int BitLoader::loadBitstreamSystem(char * binfile){
	char command[200];
	
	int err;
	err = system ("echo 0 | sudo tee /sys/class/fpga_manager/fpga0/flags");
	if(err == -1){
		printf("Error in system function 1\n");
		return -1;
	}
	err = system ("sudo mkdir -p /lib/firmware");
	if(err == -1){
		printf("Error in system function 2\n");
		return -1;
	}
	sprintf(command, "sudo cp %s /lib/firmware/",binfile);
	err = system (command);
	if(err == -1){
		printf("Error in system function 3\n");
		return -1;
	}
	sprintf(command, "echo %s | sudo tee /sys/class/fpga_manager/fpga0/firmware",binfile);
	err = system (command);
	if(err == -1){
		printf("Error in system function 4\n");
		return -1;
	}

	return 0;
}
