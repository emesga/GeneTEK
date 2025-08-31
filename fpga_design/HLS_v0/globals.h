#ifndef globals_H
#define globals_H

// #define AP_INT_MAX_W 4096

// Includes
#include "math.h"
#include "hls_task.h"
#include "hls_np_channel.h"
#include <ap_int.h>

// Debug
// #define DEBUG

// Global application parameter
#define MAX_SEQ_LENGTH 360
#define MAX_SEQBITS_LENGTH (MAX_SEQ_LENGTH * 8)

// Design parameters
#define NUM_WORKERS 42
#define QUERY_BLOCK_SIZE 10240

// Stats
#define AVG_SEQ_LENGTH 150
#define AVG_NUM_QUERY (uint32_t)1e3
#define AVG_NUM_TARGETS (uint32_t)1e3
#define NUM_QUERY_BLOCKS (uint32_t)(AVG_NUM_QUERY/QUERY_BLOCK_SIZE)

// Others
#define SCORE_BITS_S 11 // (uint32_t)(ceil(log2(MAX_SEQ_LENGTH*2)) + 1)
#define POS_BITS_U 9 // (uint32_t)ceil(log2(MAX_SEQ_LENGTH))

typedef ap_uint<MAX_SEQ_LENGTH> __m512i;
#define block_l MAX_SEQBITS_LENGTH
typedef ap_uint<8> sequence_chain;
#define block_l_m MAX_SEQ_LENGTH
#define NREFERENCES_block MAX_SEQBITS_LENGTH
#define NPATTERNS_block 6
typedef ap_uint<block_l_m> pattern_list_block[NPATTERNS_block];
typedef ap_uint<32> pattern_length_block[NPATTERNS_block];

typedef ap_uint<block_l_m> sequence_chain_m;

// ref == target
// pat == query

struct msg_in_t
{
    sequence_chain_m bit1_ref; // change to vector of 1 bit
    sequence_chain_m bit2_ref;
    ap_uint<POS_BITS_U> length_ref;
    sequence_chain_m bit1_pat;
    sequence_chain_m bit2_pat;
    ap_uint<POS_BITS_U> length_pat;
    uint32_t id; // querries x targets = ...
};

struct msg_out_t
{
    ap_uint<POS_BITS_U> pos;
    uint32_t id; // querries x targets = ...
};



extern void SeqMatcherHW(sequence_chain *bit_set_target, int nseqt, uint32_t * length_target, // Target
                        sequence_chain *bit_set_query, int nseqq, uint32_t * length_query, // Querry
                        int32_t * output);

#endif
