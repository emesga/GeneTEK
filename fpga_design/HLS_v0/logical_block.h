#ifndef logicak_block_H
#define logicak_block_H

#include "globals.h"

inline __m512i _mm512_or_si512(__m512i reg1, __m512i reg2){
#pragma HLS INLINE
	return(reg1 | reg2);
}

inline __m512i _mm512_xor_si512(__m512i reg1, __m512i reg2){
#pragma HLS INLINE
	return(reg1 ^ reg2);
}

inline __m512i _mm512_and_si512(__m512i reg1, __m512i reg2){
#pragma HLS INLINE
	return(reg1 & reg2);
}

inline bool eq(__m512i reg1, __m512i reg2){
#pragma HLS INLINE
	return(reg1 == reg2);
}

inline __m512i shift_left(__m512i reg){
#pragma HLS INLINE
	return(reg << 1);
}

inline __m512i shift_right(__m512i reg){
#pragma HLS INLINE
	return(reg >> 1);
}

inline __m512i sum(__m512i A, __m512i B){
#pragma HLS INLINE
	return A + B;
}

inline __m512i mask_pattern_2bit_v2(__m512i bit1, __m512i bit2, ap_uint<1> next_bit1_ref, ap_uint<1> next_bit2_ref){
#pragma HLS INLINE
  __m512i code1 = next_bit1_ref ? 0 : ~0;
  __m512i code2 = next_bit2_ref ? 0 : ~0;
	__m512i comp1 = code1 ^ bit1;
	__m512i comp2 = code2 ^ bit2;
	return(comp1 & comp2);
}


#endif

