/***********************************************************************************************************************************************************
*  \file       	bitloader.hpp
*  \details    	Load the bitstream to the Programable Logic through the Xilinx FPGA Manager Driver
*  \author     	Ruben Rodriguez Alvarez
*  \brief: 		For use in PYNQ-Z2 and Zynq Ultrascale+ (ZCU104)
***********************************************************************************************************************************************************/

#ifndef	BITLOADER_HPP
#define	BITLOADER_HPP

class BitLoader {
public:
    // Constructor
    BitLoader() {
        // Initialize clock here
    }

    // Destructor
    ~BitLoader() {
        // Clean up clock resources here
    }

    int loadBitstream(char * binfile);
    int loadBitstreamSystem(char * binfile);
};

#endif // BITLOADER_HPP
