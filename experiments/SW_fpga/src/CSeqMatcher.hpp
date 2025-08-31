#ifndef CSEQMATCHER_HPP
#define CSEQMATCHER_HPP

class CSeqMatcher : public CAccelDriver {
  protected:
    // Structure that mimics the layout of the peripheral registers.
    // Vitis HLS skips some addresses in the register file. We introduce
    // padding fields to create the right mapping to registers with our structure.
    struct TRegs {
      uint32_t control; // 0x00
      uint32_t gier, ier, isr; // 0x04, 0x08, 0x0C
      uint32_t bit_set_ref_1, bit_set_ref_2; // 0x10, 0x14
      uint32_t padding1; // 0x18
      uint32_t nseqt; // 0x1C
      uint32_t padding2; // 0x20
      uint32_t length_ref_1, length_ref_2; //0x24, 0x28
      uint32_t padding3; // 0x2C
      uint32_t bit_set_pat_1, bit_set_pat_2; // 0x30, 0x34
      uint32_t padding4; // 0x38
      uint32_t nseqp; // 0x3C
      uint32_t padding5; // 0x40
      uint32_t length_pat_1, length_pat_2; // 0x44, 0x48
      uint32_t padding6; // 0x4C
      uint32_t output_1, output_2; // 0x50, 0x54
    };

    // Type of waiting to the accelerator
    typedef enum {
      INTERRUPT = 0x0,  // Wait for the interrupt
      POLLING   = 0x1,  // Polling on the status register
      CONTINUE  = 0x2,  // Do not wait and return to the user
    } read_type_t;

    // Structure used to pass commands between user-space and kernel-space.
    struct write_message {
      uint64_t seq_t;         // Pointer to the target sequences
      uint64_t seq_q;         // Pointer to the query sequences
      uint32_t n_seq_t;       // Number of targets
      uint32_t n_seq_q;       // Number of queries
      uint64_t length_seq_t;  // Pointer to the target sequence lengths
      uint64_t length_seq_q;  // Pointer to the target sequence lengths
      uint64_t min_pos;       // Pointer to the min pos array
      read_type_t wait_type;  // Type of waiting to the accelerator
    };

    uint32_t GetPhyAddress(void * virtAddr, uint64_t & phyAddr);

  protected:
    static uint64_t phy_reference_c, phy_length_ref, phy_pattern_c, phy_length_pat, phy_output;
    static uint32_t max_seq_length_internal;
    static bool phy_initialized;

  public:
    CSeqMatcher(bool Logging = false)
      : CAccelDriver(Logging) {}

    ~CSeqMatcher() {}

    // Direct implementations
    uint32_t InitConfig(void * reference_c, void * length_ref,
      void * pattern_c, void * length_pat,
      void * output, int32_t max_seq_length);
    uint32_t AlignmentConfig(int32_t reference_c_off, int32_t nseqt, int32_t length_ref_off,
      int32_t pattern_c_off, int32_t nseqp, int32_t length_pat_off,
      int32_t output_off);
    uint32_t AlignmentStart();
    uint32_t AlignmentWait();

    // Driver implementation
    uint32_t AlignmentDriverConfig(int32_t reference_c_off, int32_t nseqt, int32_t length_ref_off,
      int32_t pattern_c_off, int32_t nseqp, int32_t length_pat_off,
      int32_t output_off);
    uint32_t AlignmentDriverStart();

    // Logs
    void PrintRegs();
};

#endif  // CSEQMATCHER_HPP

