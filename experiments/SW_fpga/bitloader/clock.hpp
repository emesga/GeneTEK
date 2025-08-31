/***********************************************************************************************************************************************************
*  \file       	clock.hpp
*  \details    	Program the clock frequency of the PL in the Zynq Ultrascale+ (ZCU104) and PYNQ-Z2
*  \author     	Ruben Rodriguez Alvarez
*  \brief: 		For use in PYNQ-Z2 and Zynq Ultrascale+ (ZCU104). More info in https://docs.amd.com/r/en-US/ug1087-zynq-ultrascale-registers/Overview
***********************************************************************************************************************************************************/

#ifndef	CLOCK_HPP
#define	CLOCK_HPP
#include <cstddef>

extern "C" {
#include <libxlnk_cma.h>  // Required for memory-mapping functions from Xilinx
}

class Clock {
protected:

    virtual void setPLDivs(uint8_t clk_idx, uint8_t div0, uint8_t div1) = 0;
    virtual void getPLDivs(uint8_t clk_idx, uint8_t &div0, uint8_t &div1) = 0;
    virtual float get_pl_src_clk_mhz(uint8_t clk_idx) = 0;
    virtual float get_ps_clk_mhz() = 0;

public:

    void setPLClock(uint8_t clk_idx, uint8_t div0 = 0, uint8_t div1 = 0, float clk_mhz = 0.0);
    float getPLClock(uint8_t clk_idx);
    float getPSClock();

};

class UltrascaleClock : public Clock{
private:

    // types
    typedef struct clk_bitfield_t {
        uint8_t srcsel   : 3; // Clock generator input source
        uint8_t unused0  : 5; // Unused bits
        uint8_t divisor0 : 6; // First divisor of the clock sourcer
        uint8_t unused1  : 2; // Unused bits
        uint8_t divisor1 : 6; // Second divisor of the clock source
        uint8_t unused2  : 2; // Unused bits
        uint8_t clkact   : 1; // Enable the clock output
        uint8_t unused3  : 7; // Unused bits
    } clk_bitfield_t;

    typedef struct pll_bitfield_t {
        uint8_t unused0 : 8; // Unused bits
        uint8_t fbdiv   : 7; // Feedback divisor for the PLL
        uint8_t unused1 : 1; // Unused bits
        uint8_t div2    : 1; // Divide output frequency by 2
        uint8_t unused2 : 3; // Unused bits
        uint8_t pre_src : 3; // Select the clock source for the PLL input
        uint8_t unused3 : 1; // Unused bits
        uint8_t unused4 : 8; // Unused bits
    } pll_bitfield_t;

    typedef struct arm_bitfield_t {
        uint8_t srcsel   : 3; // First divisor of the clock sourcer
        uint8_t unused0  : 5; // Unused bits
        uint8_t divisor0 : 6; // Clock generator input source
        uint8_t unused1  : 2; // Unused bits
        uint8_t unused2  : 8; // Unused bits
        uint8_t unused3  : 8; // Unused bits
    } arm_bitfield_t;

    typedef union clk_reg_t {
        uint32_t reg;
        clk_bitfield_t bitfield;
    } clk_reg_t;

    typedef union pll_reg_t {
        uint32_t reg;
        pll_bitfield_t bitfield;
    } pll_reg_t;

    typedef union arm_reg_t {
        uint32_t reg;
        arm_bitfield_t bitfield;
    } arm_reg_t;

    // Constants
    const float DEFAULT_SRC_CLK_MHZ = 33.333;
    const uint64_t CRL_APB_ADDRESS = 0xFF5E0000;
    const uint64_t CRF_APB_ADDRESS = 0xFD1A0000;
    const uint16_t BASE_SIZE = 0x100;
    const uint8_t CRX_APB_SRC_DEFAULT = 0;

    // Methods
    void setPLDivs(uint8_t clk_idx, uint8_t div0, uint8_t div1);
    void getPLDivs(uint8_t clk_idx, uint8_t &div0, uint8_t &div1);
    float get_pl_src_clk_mhz(uint8_t clk_idx);
    float get_ps_clk_mhz();
    float get_pll_clk_mhz(pll_reg_t *pll_reg);
    void setPLregs(clk_reg_t *reg, uint8_t div0, uint8_t div1, uint8_t clk_src);
    void setPLLregs(pll_reg_t *reg, uint8_t fbdiv, uint8_t div2, uint8_t pre_src);

    // Variables
    struct {
        uint8_t iopll_ctrl_off   = 0x20; // IOPLL Clock Unit Control
        uint8_t rpll_ctrl_off    = 0x30; // RPLL Clock Unit Control 
        uint8_t pl0_ref_ctrl_off = 0xc0; // PL Clock 0 Control 
        uint8_t pl1_ref_ctrl_off = 0xc4; // PL Clock 1 Control 
        uint8_t pl2_ref_ctrl_off = 0xc8; // PL Clock 2 Control 
        uint8_t pl3_ref_ctrl_off = 0xcc; // PL Clock 3 Control 
    } crl_reg_offsets;

    struct {
        uint8_t apll_ctrl_off = 0x20; // APLL Clock Unit Control
        uint8_t dpll_ctrl_off = 0x2C; // DPLL Clock Unit Control
        uint8_t vpll_ctrl_off = 0x38; // VPLL Clock Unit Control
        uint8_t acpu_ctrl_off = 0x60; // CPU Clock Control
    } crf_reg_offsets;

    void *crl_regs;
    void *crf_regs;
    clk_reg_t * pl_clk_ctrls[4];
    pll_reg_t * pl_pll_ctrls[4];
    pll_reg_t * ps_pll_ctrls[4];
    arm_reg_t * ps_clk_ctrls;

public:

    UltrascaleClock() {
        crl_regs = (void *)cma_mmap(CRL_APB_ADDRESS, BASE_SIZE);
        if ((int64_t)crl_regs == -1) {
            printf("Error mapping the peripheral address (0x%016lX)!\n", CRL_APB_ADDRESS);
            crl_regs = NULL;
        }
        crf_regs = (void *)cma_mmap(CRF_APB_ADDRESS, BASE_SIZE);
        if ((int64_t)crf_regs == -1) {
            printf("Error mapping the peripheral address (0x%016lX)!\n", CRF_APB_ADDRESS);
            crf_regs = NULL;
        }

        pl_clk_ctrls[0] = (clk_reg_t *)((char*)crl_regs + crl_reg_offsets.pl0_ref_ctrl_off);
        pl_clk_ctrls[1] = (clk_reg_t *)((char*)crl_regs + crl_reg_offsets.pl1_ref_ctrl_off);
        pl_clk_ctrls[2] = (clk_reg_t *)((char*)crl_regs + crl_reg_offsets.pl2_ref_ctrl_off);
        pl_clk_ctrls[3] = (clk_reg_t *)((char*)crl_regs + crl_reg_offsets.pl3_ref_ctrl_off);

        pl_pll_ctrls[0] = (pll_reg_t *)((char*)crl_regs + crl_reg_offsets.iopll_ctrl_off);
        pl_pll_ctrls[1] = NULL;
        pl_pll_ctrls[2] = (pll_reg_t *)((char*)crl_regs + crl_reg_offsets.rpll_ctrl_off);
        pl_pll_ctrls[3] = (pll_reg_t *)((char*)crf_regs + crf_reg_offsets.dpll_ctrl_off);

        ps_pll_ctrls[0] = (pll_reg_t *)((char*)crf_regs + crf_reg_offsets.apll_ctrl_off);
        ps_pll_ctrls[1] = NULL;
        ps_pll_ctrls[2] = (pll_reg_t *)((char*)crf_regs + crf_reg_offsets.dpll_ctrl_off);
        ps_pll_ctrls[3] = (pll_reg_t *)((char*)crf_regs + crf_reg_offsets.vpll_ctrl_off);

        ps_clk_ctrls = (arm_reg_t *)((char*)crf_regs + crf_reg_offsets.acpu_ctrl_off);
    }

    ~UltrascaleClock() {
        if (crl_regs != NULL) {
            cma_munmap((void*)crl_regs, BASE_SIZE);
        }
        crl_regs = NULL;
        if (crf_regs != NULL) {
            cma_munmap((void*)crf_regs, BASE_SIZE);
        }
    }

    void fullSetPLClock(uint8_t clk_idx, uint8_t div0, uint8_t div1, uint8_t src, uint8_t fbdiv, uint8_t div2, uint8_t pre_src);
};

class PynqClock : public Clock {
private:

    // types
    typedef struct clk_bitfield_t {
        uint8_t unused0  : 4; // Unused bits
        uint8_t srcsel   : 2; // Second divisor of the clock source
        uint8_t unused1  : 2; // Unused bits
        uint8_t divisor0 : 6; // First divisor of the clock source
        uint8_t unused2  : 6; // Unused bits
        uint8_t divisor1 : 6; // Select the source for the clock
        uint8_t unused3  : 6; // Unused bits
    } clk_bitfield_t;

    typedef struct pll_bitfield_t {
        uint8_t unused0  : 8; // Unused bits
        uint8_t unused1  : 4; // Unused bits
        uint8_t pll_fdiv : 7; // Feedback divisor for the PLL
        uint8_t unused2  : 5; // Unused bits
        uint8_t unused3  : 8; // Unused bits
    } pll_bitfield_t;

    typedef struct arm_bitfield_t {
        uint8_t unused0  : 4; // Unused bits
        uint8_t srcsel   : 2; // Frequency divisor for the CPU clock
        uint8_t unused1  : 2; // Unused bits
        uint8_t divisor  : 6; // Source for the CPU clock
        uint8_t unused2  : 2; // Unused bits
        uint8_t unused3  : 8; // Unused bits
        uint8_t unused4  : 8; // Unused bits
    } arm_bitfield_t;

    typedef union clk_reg_t {
        uint32_t reg;
        clk_bitfield_t bitfield;
    } clk_reg_t;

    typedef union pll_reg_t {
        uint32_t reg;
        pll_bitfield_t bitfield;
    } pll_reg_t;

    typedef union arm_reg_t {
        uint32_t reg;
        arm_bitfield_t bitfield;
    } arm_reg_t;

    // Constants
    const float DEFAULT_SRC_CLK_MHZ = 50.0;
    const uint32_t SLCR_BASE_ADDRESS = 0xF8000000;
    const uint16_t SLCR_BASE_SIZE = 0x200;

    // Methods
    void setPLDivs(uint8_t clk_idx, uint8_t div0, uint8_t div1);
    void getPLDivs(uint8_t clk_idx, uint8_t &div0, uint8_t &div1);
    float get_pl_src_clk_mhz(uint8_t clk_idx);
    float get_ps_clk_mhz();
    float get_pll_clk_mhz(pll_reg_t *pll_reg);

    // Variables
    struct {
        uint16_t arm_pll_ctrl_off    = 0x0100; // ARM PLL Control
        uint16_t ddr_pll_ctrl_off    = 0x0104; // DDR PLL Control
        uint16_t io_pll_ctrl_off     = 0x0108; // IO PLL Control
        uint16_t arm_clk_ctrl_off    = 0x0120; // CPU Clock Control
        uint16_t fpga0_clk_ctrl_off  = 0x0170; // PL Clock 0 Control
        uint16_t fpga1_clk_ctrl_off  = 0x0180; // PL Clock 1 Control
        uint16_t fpga2_clk_ctrl_off  = 0x0190; // PL Clock 2 Control
        uint16_t fpga3_clk_ctrl_off  = 0x01a0; // PL Clock 3 Control
    } slcr_reg_offsets;
    void *clock_regs;
    clk_reg_t * pl_clk_ctrls[4];
    pll_reg_t * pl_pll_ctrls[4];
    pll_reg_t * ps_pll_ctrls[4];
    arm_reg_t * ps_clk_ctrls;

public:

    PynqClock() {
        clock_regs = (void *)cma_mmap(SLCR_BASE_ADDRESS, SLCR_BASE_SIZE);
        if ((int64_t)clock_regs == -1) {
            printf("Error mapping the peripheral address (0x%016X)!\n", SLCR_BASE_ADDRESS);
            clock_regs = NULL;
        }

        pl_clk_ctrls[0] = (clk_reg_t *)((char*)clock_regs + slcr_reg_offsets.fpga0_clk_ctrl_off);
        pl_clk_ctrls[1] = (clk_reg_t *)((char*)clock_regs + slcr_reg_offsets.fpga1_clk_ctrl_off);
        pl_clk_ctrls[2] = (clk_reg_t *)((char*)clock_regs + slcr_reg_offsets.fpga2_clk_ctrl_off);
        pl_clk_ctrls[3] = (clk_reg_t *)((char*)clock_regs + slcr_reg_offsets.fpga3_clk_ctrl_off);

        pl_pll_ctrls[0] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.io_pll_ctrl_off);
        pl_pll_ctrls[1] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.io_pll_ctrl_off);
        pl_pll_ctrls[2] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.arm_pll_ctrl_off);
        pl_pll_ctrls[3] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.ddr_pll_ctrl_off);

        ps_pll_ctrls[0] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.arm_pll_ctrl_off);
        ps_pll_ctrls[1] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.arm_pll_ctrl_off);
        ps_pll_ctrls[2] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.ddr_pll_ctrl_off);
        ps_pll_ctrls[3] = (pll_reg_t *)((char*)clock_regs + slcr_reg_offsets.io_pll_ctrl_off);

        ps_clk_ctrls = (arm_reg_t *)((char*)clock_regs + slcr_reg_offsets.arm_clk_ctrl_off);
    }

    ~PynqClock() {
        if (clock_regs != NULL) {
            cma_munmap((void*)clock_regs, SLCR_BASE_SIZE);
        }
        clock_regs = NULL;
    }

};

#endif // CLOCK_HPP
