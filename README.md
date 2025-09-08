# GeneTek

## How to run

### Getting started

We are using the PYNQ image version 2.7.

#### Get the data
Uncompress data in the folder `experiments/data`:
```bash
tar xvzf 1000.tar.gz
tar xvzf 5000.tar.gz
tar xvzf 10000.tar.gz
tar xvzf 100000.tar.gz
```

#### Source
```bash
export PMT_LIB_PATH=/home/rrodrigu/hpc/acceleratedgenome/baseline/pmt-lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PMT_LIB_PATH/lib
```

#### Install in the ultrascale

Install the FPGA manager to be able to easily program the PL with the bitstream: 
```bash
sudo apt-get install fpga-manager-xlnx
```

#### Plotting results
Using conda or miniconda, create an environment with the following packages:
```bash
conda create -n hls_dse seaborn pandas numpy matplotlib python=3.10
conda activate hls_dse
```

#### Kernel drivers in the ultrascale
```bash
cd /lib/modules/5.15.19-xilinx-v2022.1/build
sudo make
```

The build process will not complete. The output should be:
```bash
make[1]: *** No rule to make target 'init/main.o', needed by 'init/built-in.a'.  Stop.
make: *** [Makefile:1868: init] Error 2
```

Load the kernel executing this in the `SW/driver` folder:
```bash
make
./load
```

When done with the application, do the following
```bash
./unload
```

```bash
sudo ./SW/bitloader/programOverlay <bitstream binary file> <clock index> <clock frquency>
```

#### PMT lib
Copy the [pmt library](https://git.astron.nl/RD/pmt) into the `/usr/lib/` so it can be executed with sudo, and export the PMT environment variables:
```bash
sudo cp SW/pmt-lib/lib/libpmt.so /usr/lib/
```

The `pmt-lib/bin/PMT` program measures the power encapsulating the execution of the program:
```bash
./pmt-lib/bin/PMT ./seqmatcher <target.fastq> <query.fastq> <num_threads> 
```

In the case the wrapper is used, the following variables must be exported in order to measure in the FPGA:
```bash
export PMT_NAME="xilinx"
export PMT_DEVICE="/sys/bus/i2c/drivers/pmbus/4-0043/hwmon/hwmon0/power1_input"
```

#### Loading the bitstream in the FPGA
In the `bitstream` folder, run the following Python script to load an accelerator
```bash
sudo -E python programOverlay.py design_1.bit
```

## Baselines (SOTA) — How to Reproduce
```bash
./<seqmatcher executable> <target.fq> <query.fq> <num_threads> <sets_length> <write_scores(true|false)> false true
./<Baseline executable> <target.fq> <query.fq> <num_threads> <sets_length> <write_scores(true|false)>
./<Edlib executable> <target.fq> <query.fq> <num_threads> <sets_length> <write_scores(true|false)>
```
### Intel's AVX
The folder `baseline` contains the executables for the AVX512
- `seqmatcher`: measure performance only
- `seqmatcher_intel_scores`: write scores

### FPGA
The folder `SW` contains the program for the board `seqmatcher`. The number of threads (`<num_threads>`) is ignored for this version. The executable can be recompiled by simply executing `make` in the folder.

### Script for automatic measurements
In the bash script `measure.sh`, you can set up the executable and the experiments and launch them with:
```bash
source measure.sh
```

### Troubleshooting

If no memory can be allocated, check and adjust the CMA alloc with this:
```bash
grep -i cma /proc/meminfo
```

The `measure.sh` script will ask for the password every time a computation is longer than 10 minutes.

## Citing
Reference our [GeneTEK paper](https://arxiv.org/abs/2509.01020):
```
@misc{espinosa2025geneteklowpowerhighperformancescalable,
      title={GeneTEK: Low-power, high-performance and scalable genome sequence matching in FPGAs}, 
      author={Elena Espinosa and Rubén Rodríguez Álvarez and José Miranda and Rafael Larrosa and Miguel Peón-Quirós and Oscar Plata and David Atienza},
      year={2025},
      eprint={2509.01020},
      archivePrefix={arXiv},
      primaryClass={cs.AR},
      url={https://arxiv.org/abs/2509.01020}, 
}
```
