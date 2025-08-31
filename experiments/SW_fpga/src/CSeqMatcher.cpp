#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <map>
#include "CAccelDriver.hpp"
#include "CSeqMatcher.hpp"

uint64_t CSeqMatcher::phy_reference_c = 0;
uint64_t CSeqMatcher::phy_length_ref = 0;
uint64_t CSeqMatcher::phy_pattern_c = 0;
uint64_t CSeqMatcher::phy_length_pat = 0;
uint64_t CSeqMatcher::phy_output = 0;
uint32_t CSeqMatcher::max_seq_length_internal = 0;
bool CSeqMatcher::phy_initialized = false;

///////////////////////////////////////////////////////////////////////////////
uint32_t CSeqMatcher::AlignmentWait()
{
  volatile TRegs * regs = (TRegs*)accelRegs;
  uint32_t status;
  uint64_t count = 0;

  if (accelRegs == NULL) {
    if (logging)
      printf("Error: Calling AlignmentWait() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  do {
    status = regs->control;
    ++ count;
    if (count> 500000) {
      printf("TIMEOUT!!!\n");
      break;
    }
    usleep(1);
  } while ( ( (status & 2) != 2) ); // wait until ap_done==1

  return OK;
}

///////////////////////////////////////////////////////////////////////////////
uint32_t CSeqMatcher::GetPhyAddress(void * virtAddr, uint64_t & phyAddr)
{
  // We need to obtain the physical addresses corresponding to each of the virtual addresses passed by the application.
  // The accelerator uses only the physical addresses (and only contiguous memory).
  phyAddr = GetDMAPhysicalAddr(virtAddr);
  if (phyAddr == 0) {
    if (logging)
      printf("Error: No physical address found for virtual address 0x%016lX\n", (uint64_t)virtAddr);
    return VIRT_ADDR_NOT_FOUND;
  }
  return OK;
}

///////////////////////////////////////////////////////////////////////////////
uint32_t CSeqMatcher::InitConfig(void * reference_c, void * length_ref,
      void * pattern_c, void * length_pat,
      void * output, int32_t max_seq_length)
{
  uint32_t res = OK;

  max_seq_length_internal = max_seq_length;

  if (logging)
    printf("CSeqMatcher::InitConfig("
        "reference_c=0x%016lX, length_ref=0x%016lX, "
        "pattern_c=0x%016lX, length_pat=0x%016lX, "
        "output=0x%016lX)\n", (uint64_t)reference_c, (uint64_t)length_ref,
        (uint64_t)pattern_c, (uint64_t)length_pat, (uint64_t)output);

  // Get physical addresses for the virtual addresses.
  if ( (res = GetPhyAddress(reference_c, phy_reference_c)) != OK ){
    printf("Error: Phy for ref_seq.\n");
    return res;
  }
  if ( (res = GetPhyAddress(length_ref, phy_length_ref)) != OK ){
    printf("Error: Phy for ref_len.\n");
    return res;
  }
  if ( (res = GetPhyAddress(pattern_c, phy_pattern_c)) != OK ){
    printf("Error: Phy for pat_seq.\n");
    return res;
  }
  if ( (res = GetPhyAddress(length_pat, phy_length_pat)) != OK ){
    printf("Error: Phy for pat_len.\n");
    return res;
  }
  if ( (res = GetPhyAddress(output, phy_output)) != OK ){
    printf("Error: Phy for out.\n");
    return res;
  }

  phy_initialized = true;
  
  return OK;
}

///////////////////////////////////////////////////////////////////////////////
uint32_t CSeqMatcher::AlignmentConfig(int32_t reference_c_off, int32_t nseqt, int32_t length_ref_off,
      int32_t pattern_c_off, int32_t nseqp, int32_t length_pat_off,
      int32_t output_off)
{
  volatile TRegs * regs = (TRegs*)accelRegs;
  uint64_t phy_reference_c_temp, phy_length_ref_temp, phy_pattern_c_temp, phy_length_pat_temp, phy_output_temp;

  if (logging)
    printf("CSeqMatcher::AlignmentConfig("
        "reference_c=0x%u, nseqt=%d, length_ref=0x%u, "
        "pattern_c=0x%u, nseqp=%d, length_pat=0x%u, "
        "output=0x%u)\n", reference_c_off, nseqt, length_ref_off,
        pattern_c_off, nseqp, length_pat_off, output_off);

  if (accelRegs == NULL) {
    if (logging)
      printf("Error: Calling AlignmentConfig() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  if (!phy_initialized) {
    if (logging)
      printf("Error: Calling AlignmentConfig() without initializing the physical addresses.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  phy_reference_c_temp = phy_reference_c + ((uint64_t)reference_c_off * max_seq_length_internal);
  phy_length_ref_temp = phy_length_ref + ((uint64_t)length_ref_off * 4); // the size of the length is 4 bytes
  phy_pattern_c_temp = phy_pattern_c + ((uint64_t)pattern_c_off * max_seq_length_internal);
  phy_length_pat_temp = phy_length_pat + ((uint64_t)length_pat_off * 4); // the size of the length is 4 bytes
  phy_output_temp = phy_output + ((uint64_t)output_off * 4); // the size of the output is 4 bytes

  regs->bit_set_ref_1 = (uint32_t)(phy_reference_c_temp & 0xFFFFFFFF);
  regs->bit_set_ref_2=(uint32_t)(phy_reference_c_temp >>32); 
  regs->nseqt = (uint32_t)(nseqt);
  regs->length_ref_1 = (uint32_t)(phy_length_ref_temp & 0xFFFFFFFF);
  regs->length_ref_2 = (uint32_t)(phy_length_ref_temp >> 32); 
  regs->bit_set_pat_1 = (uint32_t)(phy_pattern_c_temp & 0xFFFFFFFF);
  regs->bit_set_pat_2 = (uint32_t)(phy_pattern_c_temp >> 32);
  regs->nseqp = (uint32_t)(nseqp); 
  regs->length_pat_1 = (uint32_t)(phy_length_pat_temp & 0xFFFFFFFF);
  regs->length_pat_2 = (uint32_t)(phy_length_pat_temp >> 32);
  regs->output_1 = (uint32_t)(phy_output_temp & 0xFFFFFFFF);
  regs->output_2 = (uint32_t)(phy_output_temp >> 32);

  return OK;
}

uint32_t CSeqMatcher::AlignmentStart()
{
  volatile TRegs * regs = (TRegs*)accelRegs;
  uint32_t status;

  if (accelRegs == NULL) {
    if (logging)
      printf("Error: Calling AlignmentStart() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  if (logging)
    printf("\nStarting accel...\n");
  
  status = regs->control;
  status |= 1;  // Set to 1 ap_start
  regs->control = status;

  return OK;
}

uint32_t CSeqMatcher::AlignmentDriverConfig(int32_t reference_c_off, int32_t nseqt, int32_t length_ref_off,
      int32_t pattern_c_off, int32_t nseqp, int32_t length_pat_off,
      int32_t output_off){

  uint64_t phy_reference_c_temp, phy_length_ref_temp, phy_pattern_c_temp, phy_length_pat_temp, phy_output_temp;
  uint32_t res = OK;

  if (logging)
    printf("CSeqMatcher::AlignmentDriverConfig("
        "reference_c=0x%u, nseqt=%d, length_ref=0x%u, "
        "pattern_c=0x%u, nseqp=%d, length_pat=0x%u, "
        "output=0x%u)\n", reference_c_off, nseqt, length_ref_off,
        pattern_c_off, nseqp, length_pat_off, output_off);

  if (driver == 0) {
    if (logging)
      printf("Error: Calling AlignmentDriverConfig() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  if (!phy_initialized) {
    if (logging)
      printf("Error: Calling AlignmentDriverConfig() without initializing the physical addresses.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  phy_reference_c_temp = phy_reference_c + ((uint64_t)reference_c_off * max_seq_length_internal);
  phy_length_ref_temp = phy_length_ref + ((uint64_t)length_ref_off * 4); // the size of the length is 4 bytes
  phy_pattern_c_temp = phy_pattern_c + ((uint64_t)pattern_c_off * max_seq_length_internal);
  phy_length_pat_temp = phy_length_pat + ((uint64_t)length_pat_off * 4); // the size of the length is 4 bytes
  phy_output_temp = phy_output + ((uint64_t)output_off * 4); // the size of the output is 4 bytes

  struct write_message message ={
    phy_reference_c_temp,
    phy_pattern_c_temp,
    (uint32_t) nseqt,
    (uint32_t) nseqp,
    phy_length_ref_temp,
    phy_length_pat_temp,
    phy_output_temp,
    INTERRUPT,
  };

  int32_t readBytes = write(driver, (void *)&message, sizeof(message));
  if (readBytes != 0)
    printf("Warning! Error while programming the peripheral: %d\n", readBytes);

  return OK;
}

uint32_t CSeqMatcher::AlignmentDriverStart()
{
  if (driver == 0) {
    if (logging)
      printf("Error: Calling AlignmentDriverStart() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  if (logging)
    printf("\nStarting accel...\n");
  
  int32_t readBytes = read(driver, NULL, 0);
  return OK;
}

void CSeqMatcher::PrintRegs()
{
  printf("AccelRegs: %016lX\n", (uint64_t)accelRegs);
}

