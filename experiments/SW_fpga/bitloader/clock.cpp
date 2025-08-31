/***********************************************************************************************************************************************************
*  \file       	clock.cpp
*  \details    	Program the clock frequency of the PL in the Zynq Ultrascale+ (ZCU104) and PYNQ-Z2
*  \author     	Ruben Rodriguez Alvarez
*  \brief: 		For use in PYNQ-Z2 and Zynq Ultrascale+ (ZCU104)
***********************************************************************************************************************************************************/

#include "clock.hpp"
#include <stdexcept> // For std::invalid_argument
#include <cmath>     // For std::round

/**
 * Ultrascale+ Clock class
 */
void UltrascaleClock::setPLDivs(uint8_t clk_idx, uint8_t div0, uint8_t div1) {
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    pl_clk_reg->bitfield.clkact = 0x1;
    pl_clk_reg->bitfield.divisor0 = div0;
    pl_clk_reg->bitfield.divisor1 = div1;
}

void UltrascaleClock::getPLDivs(uint8_t clk_idx, uint8_t &div0, uint8_t &div1) {
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    div0 = pl_clk_reg->bitfield.divisor0;
    div1 = pl_clk_reg->bitfield.divisor1;
}

float UltrascaleClock::get_pl_src_clk_mhz(uint8_t clk_idx) {
    if (clk_idx < 0 || clk_idx > 3) {
        throw std::invalid_argument("Valid PLL index is 0 - 3.");
    }
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    uint8_t src_clk_idx = pl_clk_reg->bitfield.srcsel;
    if (src_clk_idx < 0 || src_clk_idx > 3) {
        throw std::invalid_argument("Valid PLL index is 0 - 3.");
    }
    pll_reg_t *pll_reg = pl_pll_ctrls[src_clk_idx];
    float clk_mhz = get_pll_clk_mhz(pll_reg);
    return clk_mhz;
}

float UltrascaleClock::get_ps_clk_mhz() {
    uint8_t arm_src_pll_idx = ps_clk_ctrls->bitfield.srcsel;
    uint8_t arm_clk_odiv = ps_clk_ctrls->bitfield.divisor0;
    pll_reg_t *pll_reg = ps_pll_ctrls[arm_src_pll_idx];
    float clk_pll_mhz = get_pll_clk_mhz(pll_reg);
    float clk_mhz = clk_pll_mhz / arm_clk_odiv;
    return clk_mhz;
}

float UltrascaleClock::get_pll_clk_mhz(pll_reg_t *pll_reg) {
    if (pll_reg == nullptr) {
        throw std::invalid_argument("Null pointer to PLL register.");
    }
    if (pll_reg->bitfield.pre_src & 0x4 != CRX_APB_SRC_DEFAULT) {
        throw std::invalid_argument("Invalid PLL Source");
    }
    uint8_t pll_fbdiv = pll_reg->bitfield.fbdiv;
    uint8_t pll_odiv2 = (pll_reg->bitfield.div2 == 1) ? 2 : 1;
    float clk_mhz = DEFAULT_SRC_CLK_MHZ * pll_fbdiv / pll_odiv2;
    return clk_mhz;
}

void UltrascaleClock::setPLregs(clk_reg_t *reg, uint8_t div0, uint8_t div1, uint8_t clk_src) {
    reg->bitfield.srcsel = clk_src;
    reg->bitfield.clkact = 0x1;
    reg->bitfield.divisor0 = div0;
    reg->bitfield.divisor1 = div1;
}

void UltrascaleClock::setPLLregs(pll_reg_t *reg, uint8_t fbdiv, uint8_t div2, uint8_t pre_src) {
    reg->bitfield.fbdiv = fbdiv;
    reg->bitfield.div2 = div2;
    reg->bitfield.pre_src = pre_src;
}

void UltrascaleClock::fullSetPLClock(uint8_t clk_idx, uint8_t div0, uint8_t div1, uint8_t clk_src, uint8_t fbdiv, uint8_t div2, uint8_t pre_src){
    if (clk_src < 0 || clk_src > 3) {
        throw std::invalid_argument("Valid PLL index is 0 - 3.");
    }
    if (clk_idx < 0 || clk_idx > 3) {
        throw std::invalid_argument("Valid PL index is 0 - 3.");
    }
    pll_reg_t *pll_reg = pl_pll_ctrls[clk_src];
    setPLLregs(pll_reg, fbdiv, div2, pre_src);
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    setPLregs(pl_clk_reg, div0, div1, clk_src);
}

/**
 * Pynq Clock class
 */
void PynqClock::setPLDivs(uint8_t clk_idx, uint8_t div0, uint8_t div1) {
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    pl_clk_reg->bitfield.divisor0 = div0;
    pl_clk_reg->bitfield.divisor1 = div1;
}

void PynqClock::getPLDivs(uint8_t clk_idx, uint8_t &div0, uint8_t &div1) {
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    div0 = pl_clk_reg->bitfield.divisor0;
    div1 = pl_clk_reg->bitfield.divisor1;
}

float PynqClock::get_pl_src_clk_mhz(uint8_t clk_idx) {
    if (clk_idx < 0 || clk_idx > 3) {
        throw std::invalid_argument("Valid PLL index is 0 - 3.");
    }
    clk_reg_t *pl_clk_reg = pl_clk_ctrls[clk_idx];
    uint8_t src_clk_idx = pl_clk_reg->bitfield.srcsel;
    if (src_clk_idx < 0 || src_clk_idx > 3) {
        throw std::invalid_argument("Valid PLL index is 0 - 3.");
    }
    pll_reg_t *pll_reg = pl_pll_ctrls[src_clk_idx];
    float clk_mhz = get_pll_clk_mhz(pll_reg);
    return clk_mhz;
}

float PynqClock::get_ps_clk_mhz() {
    uint8_t arm_src_pll_idx = ps_clk_ctrls->bitfield.srcsel;
    uint8_t arm_clk_odiv = ps_clk_ctrls->bitfield.divisor;
    pll_reg_t *pll_reg = ps_pll_ctrls[arm_src_pll_idx];
    float clk_pll_mhz = get_pll_clk_mhz(pll_reg);
    float clk_mhz = clk_pll_mhz / arm_clk_odiv;
    return clk_mhz;
}

float PynqClock::get_pll_clk_mhz(pll_reg_t *pll_reg) {
    if (pll_reg == nullptr) {
        throw std::invalid_argument("Null pointer to PLL register.");
    }
    uint8_t pll_fdiv = pll_reg->bitfield.pll_fdiv;
    float clk_mhz = DEFAULT_SRC_CLK_MHZ * pll_fdiv;
    return clk_mhz;
}

/**
 * Clock class
 */
void Clock::setPLClock(uint8_t clk_idx, uint8_t div0, uint8_t div1, float clk_mhz) {
    if (clk_idx < 0 || clk_idx > 3) {
        throw std::invalid_argument("Valid PL clock index is 0 - 3.");
    }

    const uint8_t div0_max = 63;
    const uint8_t div1_max = 63;

    float src_clk_mhz = get_pl_src_clk_mhz(clk_idx);
    uint16_t product = std::ceil(src_clk_mhz / clk_mhz); // Will select the frequency extricly lower than the desired one

    if (div0 == 0 && div1 == 0) {
        // Algorith to find the divisors
        uint8_t div0_min = std::ceil(div1_max / product);
        uint16_t dist = std::abs(product - div0_min * div1_max);
        uint16_t min_dist = dist;
        div0 = div0_min;
        div1 = div1_max;
        while(dist != 0){
            uint16_t div1_temp = std::ceil(product / div0_min);
            dist = std::abs(product - div0_min * div1_temp);
            if (dist < min_dist) {
                div0 = div0_min;
                div1 = div1_temp;
            }
            div0_min++;
            if (div0_min >= div0_max) {
                break;
            }
        };
    } else if (div0 != 0 && div1 == 0) {
        div1 = std::ceil(product / div0);
    } else if (div1 != 0 && div0 == 0) {
        div0 = std::ceil(product / div1);
    }

    if (div0 <= 0 || div0 > div0_max) {
        throw std::invalid_argument("Frequency divider 0 value out of range.");
    }
    if (div1 <= 0 || div1 > div1_max) {
        throw std::invalid_argument("Frequency divider 1 value out of range.");
    }

    setPLDivs(clk_idx, div0, div1);
}
float Clock::getPLClock(uint8_t clk_idx) {
    if (clk_idx < 0 || clk_idx > 3) {
        throw std::invalid_argument("Valid PL clock index is 0 - 3.");
    }

    uint8_t pl_clk_odiv0, pl_clk_odiv1;
    float src_clk_mhz = get_pl_src_clk_mhz(clk_idx);
    getPLDivs(clk_idx, pl_clk_odiv0, pl_clk_odiv1);

    // Calculate the clock frequency
    float pl_clk_mhz = src_clk_mhz / (pl_clk_odiv0 * pl_clk_odiv1);

    return pl_clk_mhz;
}

float Clock::getPSClock() {
    return get_ps_clk_mhz();
}
