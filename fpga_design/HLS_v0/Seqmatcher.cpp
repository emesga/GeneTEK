#include <algorithm>
#include <iostream>
#include <vector>
#include <numeric>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <iterator>
#include <stdint.h>

#include "globals.h"
#include "logical_block.h"

/**
 * TODO
 * PRIMARY
 * - (E&R) One worker, several queries
 * SECONDARY
 * - (E&R) Inster golden reference in SW
 * - (E&R) Try array of 1 bit (do the luts map better?)
 * - (E&R) Use memories in string matching to reduce FF (with high frequencies)
 * - (E&R) Buffer length x number of sequences
 */

///////////////////////////////////////////////////////////////////////////////
/**
 * Translates 8 to 2 bits on the fly
 */
void bit_process(sequence_chain *seq, ap_uint<32> seq_offset, ap_uint<POS_BITS_U> T_sequence,  sequence_chain_m &bit1,  sequence_chain_m &bit2)
{
#pragma HLS INLINE
	// Reading and translating by blocks 
	sequence_chain seq_temp;
	translate_loop: for(int i=0; i<T_sequence; i++){
	#pragma HLS LOOP_TRIPCOUNT avg=AVG_SEQ_LENGTH max=MAX_SEQ_LENGTH
	//#pragma HLS unroll factor=16
		seq_temp = seq[seq_offset + i];
		bit1.range(i,i) = seq_temp.range(1,1);
		bit2.range(i,i) = seq_temp.range(2,2);
	}

	#ifdef DEBUG
	printf("String at : %016X\n", seq);
	for (int i = 0; i < T_sequence; i++) {
		std::cout << seq[i];
		std::cout << "|";
	}
	std::cout << std::endl;
	std::cout << "String2a" << std::endl;
	for (int i = 0; i < T_sequence; i++) {
		std::cout << (((uint8_t)bit2>>i)&1) << "." << (((uint8_t)bit1>>i)&1);
		std::cout << "|";
	}
	std::cout << std::endl;
	#endif
}

///////////////////////////////////////////////////////////////////////////////
/**
 * Read module with buffer
 */
void read_in(
	sequence_chain *seq_target, int nseq_target, uint32_t * length_target,
	sequence_chain *seq_query, int nseq_query, uint32_t * length_query,
	hls::stream<msg_in_t>& worker_in) {
  
  	uint32_t * p_lengths_target, * p_lengths_query = length_query;
	ap_uint<32> seq_query_offset = 0, seq_target_offset = 0;

	msg_in_t in;
	in.id = 0;

	uint32_t remainingQuerries;
	int32_t block;

	sequence_chain_m block_query[QUERY_BLOCK_SIZE][2]; // all chain over 2 bits
	ap_uint<POS_BITS_U> block_length_query[QUERY_BLOCK_SIZE];

	iter_Q_block: for (int j = 0; j < nseq_query; j+=QUERY_BLOCK_SIZE) {
	#pragma HLS LOOP_TRIPCOUNT avg=NUM_QUERY_BLOCKS max=NUM_QUERY_BLOCKS

		p_lengths_target = length_target;
		seq_target_offset = 0;
		remainingQuerries = nseq_query - j;
		block = remainingQuerries > QUERY_BLOCK_SIZE ? QUERY_BLOCK_SIZE : remainingQuerries;
		in.id = j;

		// Save Target in table
		read_Q_block: for (int s = 0; s < block; s++) {
		#pragma HLS LOOP_TRIPCOUNT avg=QUERY_BLOCK_SIZE max=QUERY_BLOCK_SIZE

			ap_uint<POS_BITS_U> l_query = (ap_uint<POS_BITS_U>)*p_lengths_query;
			block_query[s][0] = 0;
			block_query[s][1] = 0;
			bit_process(seq_query, seq_query_offset, l_query, block_query[s][0], block_query[s][1]);
			seq_query_offset += MAX_SEQ_LENGTH;
			block_length_query[s] = l_query; // Pass to the next sequence in the target
			p_lengths_query++;

		}

		// Compare all references/targets entries with the block of specimen entries.
		iter_ref: for (int i = 0; i < nseq_target; i++) {
		#pragma HLS LOOP_TRIPCOUNT avg=AVG_NUM_TARGETS max=AVG_NUM_TARGETS

			in.length_ref = (ap_uint<POS_BITS_U>)*p_lengths_target;
			bit_process(seq_target, seq_target_offset, in.length_ref, in.bit1_ref, in.bit2_ref);
			seq_target_offset += MAX_SEQ_LENGTH;
			p_lengths_target++;  // Pass to the next sequence in the DB

			// For each DB entry, compare with all specimens in the block
			copy_Q_block: for (int s = 0; s < block; s++) {
			#pragma HLS LOOP_TRIPCOUNT avg=QUERY_BLOCK_SIZE max=QUERY_BLOCK_SIZE

				in.bit1_pat = block_query[s][0];
				in.bit2_pat = block_query[s][1];
				in.length_pat = block_length_query[s];

				worker_in.write(in);
				in.id++;

			}

			in.id += nseq_query - block;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 * Write module
 */
void write_out(hls::stream<msg_out_t>& worker_out, int32_t * output, uint32_t nseqt, uint32_t nseqq) {

  write_queries: for (int j = 0; j < nseqq; j++) {
  #pragma HLS LOOP_TRIPCOUNT avg=AVG_NUM_QUERY max=AVG_NUM_QUERY

    write_targets: for (int i = 0; i < nseqt; i++) {
    #pragma HLS LOOP_TRIPCOUNT avg=AVG_NUM_TARGETS max=AVG_NUM_TARGETS

      msg_out_t out = worker_out.read();
      output[out.id] = (int32_t)out.pos;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/**
 * Module to compare sequences
 */
void String_matching( hls::stream<msg_in_t>& msg_in, hls::stream<msg_out_t>& msg_out )
{
  // Read from FIFO
  msg_in_t in = msg_in.read();

  ap_int<SCORE_BITS_S> score_list = in.length_pat, min_value = score_list;
  ap_int<POS_BITS_U> min_pos = 0;
  // Assume a maximum size of 300 bases and that they can be processed in one go
  // Create and initialize all bit vectors
  sequence_chain_m D0, HN, HP, X, VN = 0, VP = ~0;
  // Initialize as well complementary bit vectors.
  sequence_chain_m mask;

  // Load the reference or target sequence, which has to be traversed one nucleotide at a time
  comp_cells: for(int j = 0; j < in.length_ref; j++) {
    #pragma HLS LOOP_TRIPCOUNT avg=AVG_SEQ_LENGTH max=MAX_SEQ_LENGTH
    #pragma HLS PIPELINE II=1

    mask = mask_pattern_2bit_v2(in.bit1_pat, in.bit2_pat, in.bit1_ref.range(j, j), in.bit2_ref.range(j, j));
    X = _mm512_or_si512(mask, VN);
    D0 = _mm512_or_si512(_mm512_xor_si512( sum(_mm512_and_si512(X, VP), VP), VP), X);
    HN = _mm512_and_si512(D0, VP);
    HP = _mm512_or_si512(VN, ~(_mm512_or_si512(D0, VP)));
    score_list += HP.range(in.length_pat-1, in.length_pat-1) - HN.range(in.length_pat-1, in.length_pat-1);
    if (score_list < min_value) {
      min_value = score_list;
      min_pos = j;
    }
    X = shift_left(HP);
    VN = _mm512_and_si512(X, D0);
    VP = _mm512_or_si512(shift_left(HN), ~(_mm512_or_si512(X, D0)));
  }

  // Copy output
  msg_out_t out;
  out.pos = min_pos;
  out.id = in.id;
  msg_out.write(out);
}

///////////////////////////////////////////////////////////////////////////////
/**
 * Top module
 */
void SeqMatcherHW(
  sequence_chain *bit_set_target, int nseqt, uint32_t * length_target, // Target
  sequence_chain *bit_set_query, int nseqq, uint32_t * length_query, // Querry
  int32_t * output)
{
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=nseqt
#pragma HLS INTERFACE s_axilite port=nseqq
#pragma HLS INTERFACE mode=m_axi depth=1024 port=bit_set_target offset=slave bundle=port_t
#pragma HLS INTERFACE mode=m_axi depth=1024 port=length_target offset=slave bundle=port_t
#pragma HLS INTERFACE mode=m_axi depth=1024 port=bit_set_query offset=slave bundle=port_q
#pragma HLS INTERFACE mode=m_axi depth=1024 port=length_query offset=slave bundle=port_q
#pragma HLS INTERFACE mode=m_axi depth=1024 num_write_outstanding=32 port=output offset=slave bundle=port_t latency=20

  hls_thread_local hls::split::round_robin<msg_in_t, NUM_WORKERS> split1;
  hls_thread_local hls::merge::round_robin<msg_out_t, NUM_WORKERS, NUM_WORKERS> merge1;
  hls_thread_local hls::task t[NUM_WORKERS];
  #pragma HLS dataflow

  read_in(bit_set_target, nseqt, length_target, bit_set_query, nseqq, length_query, split1.in);

  workers_loop: for (int i = 0; i < NUM_WORKERS; i++) {
  #pragma HLS unroll
    t[i](String_matching, split1.out[i], merge1.in[i]);
  }

  write_out(merge1.out, output, nseqt, nseqq);
}

