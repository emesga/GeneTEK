/***********************************************************************************************************************************************************
*  \file       	programOverlay.cpp
*  \details    	Program the PL with a bitstream and set the clock frequency
*  \author     	Ruben Rodriguez Alvarez
*  \brief: 		For use in PYNQ-Z2 or Zynq Ultrascale+ (ZCU104)
***********************************************************************************************************************************************************/

#include <iostream>
#include <string>
#include <sys/utsname.h>
#include "bitloader.hpp"
#include "clock.hpp"
#include "parser.hpp"

#if defined(IS_ZCU104_ARCH)
    #pragma message("Compiling for the Zynq Ultrascale+ ZCU104")
#elif defined(IS_PYNQZ2_ARCH)
    #pragma message("Compiling for the PYNQ")
#else
    #pragma error("Architecture not supported")
#endif

#if defined(IS_PYNQZ2_ARCH) && defined(IS_ZCU104_ARCH)
    #pragma error("Cannot compile for both architectures")
#endif

/**
 * Boards supported
 */
const std::string PYNQZ2_ARCH = "armv7l";
const std::string ZCU104_ARCH = "aarch64";

/**
 * Load bitstream implementation
 * false: File operations
 * true: system call
 */
#define SYS_CALL false

// Function to get CPU architecture
std::string get_cpu_arch() {
    struct utsname unameData;
    uname(&unameData);
    return std::string(unameData.machine);
}

int main(int argc, char* argv[]) {
    // Check the machine architecture
    std::string machine_arch = get_cpu_arch();
    // assert if the machine is supported
    if (machine_arch == PYNQZ2_ARCH) {
        #ifndef IS_PYNQZ2_ARCH
        std::cerr << "Machine architecture missmatch. This program was not compiled for this machine" << std::endl;
        return 1;
        #endif
    } else if (machine_arch == ZCU104_ARCH) {
        #ifndef IS_ZCU104_ARCH
        std::cerr << "Machine architecture missmatch. This program was not compiled for this machine" << std::endl;
        return 1;
        #endif
    } else {
        std::cerr << "Machine architecture: " << machine_arch << std::endl;
        std::cerr << "This program is only supported on PYNQ-Z2 and Zynq Ultrascale+ (ZCU104)" << std::endl;
        return 1;
    }

    // check the bitstream was passed
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " <bit file> <hwh file> <clock index> <clock frquency> <verbose>" << std::endl;
        return 1;
    }

    #if IS_PYNQZ2_ARCH
    // Load the bitstream
    BitLoader bitLoader;
    #if SYS_CALL
    bitLoader.loadBitstreamSystem(argv[1]);
    #else 
    bitLoader.loadBitstream(argv[1]);
    #endif
    #endif


    // Set the clock frequency
    #if IS_ZCU104_ARCH

        // Map to store the keys and their extracted values
        std::unordered_map<std::string, std::string> keysMap;
        // Parse the design_1.hwh file to extract the key-value pairs using keys.json as reference
        parseFile(argv[2], "SW_fpga/bitloader/keys.json", keysMap);

        UltrascaleClock clk;
        clock_regs_t regs[4];
        bool found = false;

        getRegs( regs, keysMap );

        for (int i = 0 ; i < 4 ; i++){
            if (regs[i].en){
                if (argv[5]){
                    printf("regs[%d].en: %d\n", i, regs[i].en);
                    printf("regs[%d].clk_idx: %d\n", i, regs[i].clk_idx);
                    printf("regs[%d].div0: %d\n", i, regs[i].div0);
                    printf("regs[%d].div1: %d\n", i, regs[i].div1);
                    printf("regs[%d].clk_src: %d\n", i, regs[i].clk_src);
                    printf("regs[%d].fbdiv: %d\n", i, regs[i].fbdiv);
                    printf("regs[%d].div2: %d\n", i, regs[i].div2);
                    printf("regs[%d].pre_src: %d\n", i, regs[i].pre_src);
                }
                found = true;
                //clk.fullSetPLClock(regs[i].clk_idx, regs[i].div0, regs[i].div1, regs[i].clk_src, regs[i].fbdiv, regs[i].div2, regs[i].pre_src);
		clk.setPLClock(regs[i].clk_idx, regs[i].div0, regs[i].div1, 0);
                if (argv[5]){
                    float myclk = clk.getPLClock(std::atoi(argv[2]));
                    std::cout << "Setting clock " << std::atoi(argv[2]) << " at " << std::stof(argv[3]) << " getting " << myclk << std::endl;
                }
            }
        }
        if (found) return 0;

        clk.setPLClock(std::atoi(argv[3]), 0, 0, std::stof(argv[4]));
        float myclk = clk.getPLClock(std::atoi(argv[2]));
        std::cout << "Setting clock " << std::atoi(argv[2]) << " at " << std::stof(argv[3]) << " getting " << myclk << std::endl;

        return 0;
    #elif IS_PYNQZ2_ARCH
        PynqClock clk;
        clk.setPLClock(std::atoi(argv[3]), 0, 0, std::stof(argv[4]));
        float myclk = clk.getPLClock(std::atoi(argv[2]));
        std::cout << "Setting clock " << std::atoi(argv[2]) << " at " << std::stof(argv[3]) << " getting " << myclk << std::endl;
    #else
        std::cerr << "The clock was not configured!" << std::endl;
        return 1;
    #endif

    return 0;
}
