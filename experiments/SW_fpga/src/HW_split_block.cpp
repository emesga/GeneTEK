#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <map>
#include <iostream>
#include "pmt.h"
#include <unistd.h>
#include "util.h"
#include "CAccelDriver.hpp"
#include "CSeqMatcher.hpp"

#define USE_DRIVER (true)
#define LOGGING (false)
#define MAX_MODULES 1
#define MIN_EXEC_TIME 100 // in seconds
const char * driver_name = "/dev/seqdriver";
const uint32_t BASE_ADDR = 0x00A0000000;
const uint64_t MAX_CMA_MALLOC = 420e6; // In Bytes. (grep -i cma /proc/meminfo)
CSeqMatcher seqMatchers;
uint64_t current_alloc = 0;

#define MAX_SEQ_LENGTH 360
#define MAX_DESCRIPTION_LENGTH 724
#define BUFFER_SIZE (MAX_SEQ_LENGTH + MAX_DESCRIPTION_LENGTH)

typedef struct {
  char *sequences, *descriptions;
  int32_t *length;
} SetSequences;

///////////////////////////////////////////////////////////////////////////////
SetSequences* read_file(const char *path, const uint32_t MAX_SEQUENCES) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    perror("Error al abrir el archivo");
    exit(EXIT_FAILURE);
  }

  bool ignore = false;
  char buffer[BUFFER_SIZE];

  SetSequences * customData = (SetSequences *)malloc(sizeof(SetSequences));
  if (customData == NULL) {
    printf("Error allocating memory for customData\n");
    return NULL;
  }

  customData->descriptions = (char*)CSeqMatcher::AllocDMACompatible(MAX_SEQUENCES * MAX_DESCRIPTION_LENGTH * sizeof(char));
  customData->sequences = (char*)CSeqMatcher::AllocDMACompatible(MAX_SEQUENCES * MAX_SEQ_LENGTH * sizeof(char));
  customData->length = (int32_t*)CSeqMatcher::AllocDMACompatible(MAX_SEQUENCES * sizeof(int32_t));
  if ( (customData->descriptions == NULL) || (customData->sequences == NULL) ||
        (customData->length == NULL) ) {
    printf("Error allocating DMA memory.\n");
    free(customData);
    return NULL;
  }
  current_alloc += MAX_SEQUENCES * MAX_SEQ_LENGTH * sizeof(char);
  current_alloc += MAX_SEQUENCES * sizeof(int32_t);

  for (int32_t i = 0; i < MAX_SEQUENCES; ++i) {
    *(customData->descriptions + i * MAX_DESCRIPTION_LENGTH) = '\0';
    *(customData->sequences + i * MAX_SEQ_LENGTH) = '\0';
    customData->length[i] = 0;
  }


  int32_t sequenceCount = 0;
  int32_t cnt=0;
  while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
    if (strncmp(buffer, "@T", 2) == 0) {
      if (sequenceCount < MAX_SEQUENCES) {
        for (uint32_t jj = 0; jj < MAX_SEQ_LENGTH; ++jj) {
          *(customData->descriptions+sequenceCount*MAX_DESCRIPTION_LENGTH+jj)=buffer[jj];
          if (buffer[jj] == '\0')
            break;
        }
        ignore=false;
      }
      else {
        break; 
      }
      sequenceCount++;
    }
    else if (buffer[0] == '+') {
      ignore=true;
    }
    else {
      if(ignore==false){
        cnt=0;
        for (uint32_t jj = 0; jj < MAX_SEQ_LENGTH; ++jj) {
          if(buffer[jj] != '\n') {
            *(customData->sequences+(sequenceCount-1)*MAX_SEQ_LENGTH+jj)=buffer[jj];
            //printf("%c", buffer[jj]);
            if (buffer[jj] == '\0')
              break;
            cnt++;
          }
        }
        // printf("\n");
        *(customData->length + sequenceCount - 1) = cnt;
      }
    }
  }
  fclose(file);

  CSeqMatcher::FreeDMACompatible(customData->descriptions);

  return customData;
}

///////////////////////////////////////////////////////////////////////////////
inline int32_t min(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}

///////////////////////////////////////////////////////////////////////////////
void split_block(SetSequences *seq_target, SetSequences *seq_query, int32_t nt, int32_t nq) {
  FILE * fp;
  struct timespec start, end;
  pmt::State pmt_start, pmt_end;
  std::unique_ptr<pmt::PMT> sensor(pmt::xilinx::Xilinx::Create(pmt::xilinx::Xilinx::ultrascale_ZCU104().c_str()));
  uint32_t res = CSeqMatcher::OK;
  uint32_t * output = 0;
  uint64_t time;
  double power;
  double energy;
  uint32_t repetitions;
  int32_t qSize;
  
  // Calculate the total number of computations
  int64_t availableMemory = MAX_CMA_MALLOC - current_alloc;
  if (availableMemory < 0) {
    printf("Error: Memory maximum was already exceeded. Aborting.\n");
    return;
  }
  int64_t maxComputations = floor(availableMemory / sizeof(uint32_t));
  if (nt > maxComputations) {
    printf("Error: The number of targets exceeds the maximum number of computations. Aborting.\n");
    return;
  }
  qSize = floor(maxComputations / nt); // maximum

  // Check if the total size exceeds the maximum
  if (qSize >= nq) {
    qSize = nq;
  } else {
    printf("Warning: The total number of computations exceeds the maximum memory allocation. The computation will be divided into chunks.\n");
    printf("Current allocation: %lu Bytes\n", current_alloc);
    printf("Available Memory: %lu Bytes\n", availableMemory);
    printf("Number of queries per chunk: %d / %d\n", qSize, nq);
  }
  
  // Allocate memory for the output
  output = (uint32_t*)CSeqMatcher::AllocDMACompatible(nt * qSize * sizeof(uint32_t));  
  if (output == NULL) {
    printf("Error allocating DMA memory for output.\n");
    return;
  }
  for (int32_t i = 0; i < nt * qSize; ++i)
  	output[i] = 27334;

  res = seqMatchers.InitConfig( seq_target->sequences, seq_target->length, seq_query->sequences, seq_query->length, output, MAX_SEQ_LENGTH);
  if (res != CSeqMatcher::OK) {
    printf("Error in the InitConfig of the accelerator.\n");
    CSeqMatcher::FreeDMACompatible(output);
    return;
  }

  // HW execution and measurement of the minimum set only (warmup)
  if ( nt < 100000 ) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    #if USE_DRIVER
    seqMatchers.AlignmentDriverConfig( 0, nt, 0, 0, qSize, 0, 0);
    seqMatchers.AlignmentDriverStart();
    #else
    seqMatchers.AlignmentConfig( 0, nt, 0, 0, nq, 0, 0);
    seqMatchers.AlignmentStart();
    seqMatchers.AlignmentWait();
    #endif
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    // Calculate the number of repetitions
    time = CalcTimeDiff(end, start);
    time = time * floor(nq / qSize); // Scale the computation to the total duration
    repetitions = ceil(MIN_EXEC_TIME / (time/1e9));
  } else {
    repetitions = 1;
  }

  if(LOGGING) 
    printf("Time reported: %lu ns. Executing %u times\n", time, repetitions);

  // HW execution and measurement (measure)
  pmt_start = sensor->Read();
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  for (int i = 0 ; i < repetitions ; i++ ) {
    #if USE_DRIVER
    for (int qid = 0 ; qid < (nq - qSize + 1) ; qid+=qSize ) {
      seqMatchers.AlignmentDriverConfig( 0, nt, 0, qid, qSize, qid, 0 );
      seqMatchers.AlignmentDriverStart();
    }
    if (nq % qSize > 0) { // Do the remaining queries if needed
      seqMatchers.AlignmentDriverConfig( 0, nt, 0, nq - nq % qSize, 182, nq - nq % qSize, 0 );
      seqMatchers.AlignmentDriverStart();
    }
    #else
    for (int qid = 0 ; qid < (nq - qSize + 1) ; qid+=qSize ) {
      seqMatchers.AlignmentConfig( 0, nt, 0, qid, qSize, qid, 0 );
      seqMatchers.AlignmentStart();
      seqMatchers.AlignmentWait();
    }
    if (nq % qSize > 0) { // Do the remaining queries if needed
      seqMatchers.AlignmentConfig( 0, nt, 0, nq - nq % qSize, 182, nq - nq % qSize, 0 );
      seqMatchers.AlignmentStart();
      seqMatchers.AlignmentWait();
    }
    #endif
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  pmt_end = sensor->Read();

  time = CalcTimeDiff(end, start);
  time = time / repetitions;
  fp = fopen ("times.txt", "a");
  fprintf(fp,"%lu\n", time);
  fclose (fp);

  power = sensor->watts(pmt_start, pmt_end);
  // We add a constant 2.25W to the power due to the second power sensor on the UltraScale+ ZCU104.
  // This secondary sensor measures the utils outside the SoC such as the LDO.
  // We tested that this sensor does not vary during the execution of any application.
  energy = (power + 2.25) * ((double)time/1e9);
  fp = fopen ("energy.txt", "a");
  fprintf(fp,"%lf\n", energy);
  fclose (fp);

  fp = fopen("scores.bin", "wb");
  fwrite(output, sizeof(uint32_t), nt * qSize, fp);
  fclose(fp);

  if(LOGGING) {
    std::cout<<"PMT stats:"<<std::endl;
    std::cout<<sensor->joules(pmt_start, pmt_end) << "[J]" << std::endl;
    std::cout<<sensor->watts(pmt_start, pmt_end) << "[W]" << std::endl;
    std::cout<<sensor->seconds(pmt_start, pmt_end) << "[s]" << std::endl;

    printf("Total time: %lu ns\n", time);

    printf("OUTPUT VALUES (nt*nq=%d):\n", nt*nq);
    for(int32_t i = 0; i <5; i++) {
      printf("%u ", output[i]);
    }
    printf("\n");
  }

  CSeqMatcher::FreeDMACompatible(output);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char const *argv[]) {
  SetSequences *seq_target=0, *seq_query=0;  
  uint64_t address_sequences_p, address_sequences_t, address_descriptions_p,
    address_descriptions_t, address_length_p, address_length_t;

  CSeqMatcher::SetLogging(false);
  #if USE_DRIVER
  if ( seqMatchers.OpenDriver(driver_name) != CAccelDriver::OK ) {
    printf("Error opening accelerator!\n");
    return -1;
  }
  #else
  if ( seqMatchers.Open(BASE_ADDR) != CAccelDriver::OK ) {
    printf("Error opening accelerator!\n");
    return -1;
  }
  #endif

  const char* target = argv[1];
  const char* query = argv[2];
  int nq = atoi(argv[3]);
  int nt = atoi(argv[4]);
  seq_target = read_file(target, nt);
  seq_query = read_file(query, nq);

  if ( (seq_target == NULL) || (seq_query == NULL) ) {
    printf("Error reading seq_target or seq_query\n");
  }
  else {
    split_block(seq_target, seq_query, nt, nq);
  }

  CSeqMatcher::FreeDMACompatible(seq_target->sequences);
  // CSeqMatcher::FreeDMACompatible(seq_target->descriptions);
  CSeqMatcher::FreeDMACompatible(seq_target->length);
  CSeqMatcher::FreeDMACompatible(seq_query->sequences);
  // CSeqMatcher::FreeDMACompatible(seq_query->descriptions);
  CSeqMatcher::FreeDMACompatible(seq_query->length);
  seqMatchers.CloseDriver();

  if (seq_target != NULL)
    free(seq_target);
  if (seq_query != NULL)
    free(seq_query);

  return 0;
}

