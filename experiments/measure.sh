#!/bin/bash

# Check if the script is run with sudo
if [ -z "$SUDO_USER" ]; then
    echo "This script must be run with sudo."
    exit 1
fi

SEQ_MAX_ACCEL=0
TARGET_FREQ_MHZ=0
NUM_WORKERS=0


# Assuming the first argument to the script is the path to the JSON config file
config_file="$1"

# Check if the JSON config file path is provided and the file exists
if [[ -z "$config_file" || ! -f "$config_file" ]]; then
	echo "Config file is missing or does not exist."
else
	# Execute the Python script and read its output into variables
	IFS=$';' read -r executable num_threads_p lengths_p nset_p vaccel_p <<< $(python3 parse_config.py "$config_file")

	# Convert comma-separated strings back into arrays (if needed)
	IFS=',' read -r -a num_threads <<< "$num_threads_p"
	IFS=',' read -r -a lengths <<< "$lengths_p"
	IFS=',' read -r -a nset <<< "$nset_p"
	IFS=',' read -r -a vaccel <<< "$vaccel_p"

	# Configuration
	echo "Executable: $executable"
	echo "Number of threads: ${num_threads[*]}"
	echo "Lengths: ${lengths[*]}"
	echo "NSet: ${nset[*]}"
	echo "VAccel: ${vaccel[*]}"

	benchmark="benchmark.csv"
	[ -f "benchmark.csv" ] && rm "benchmark.csv"
	[ -f "times.txt" ] && rm "times.txt"
	[ -f "energy.txt" ] && rm "energy.txt"

	if [[ "$executable" == "./SW_fpga/seqmatcher" ]]; then
		cd SW_fpga/driver
		./load
		cd ../..
	fi

	echo "Number of sequences,String lengths,time_ns,energy_J,Accel ID,Threads,Time Start,Time End" > "$benchmark"
	for acc in "${vaccel[@]}"; do
		if [[ "$executable" == "./SW_fpga/seqmatcher" ]]; then
			# fpgautil -b bitstream/accel_v$acc/design_1.bit
			echo 0 > /sys/class/fpga_manager/fpga0/flags # Full bitstream
			mkdir -p /lib/firmware
			cp bitstream/accel_v$acc/design_1.bit.bin /lib/firmware/
			echo design_1.bit.bin > /sys/class/fpga_manager/fpga0/firmware # Load bitstream
			source bitstream/accel_v$acc/accel.sh
			./SW_fpga/bitloader/programOverlay bitstream/accel_v$acc/design_1.bit bitstream/accel_v$acc/design_1.hwh 0 $TARGET_FREQ_MHZ 1
		fi
		for nth in "${num_threads[@]}"; do
			for n in "${nset[@]}"; do
				for s in $(seq 0 $(( ${#lengths[@]} - 1 ))); do
					#if [[ $SEQ_MAX_ACCEL -ge ${lengths[s]} ]]; then
						if [[ -f "data/$n/${lengths[s]}.fq" && -s "data/$n/${lengths[s]}.fq" ]]; then
              rm -f Scores.bin
							time_start=$(date +%s)
							if [[ "$executable" == "./SW_fpga/seqmatcher" ]]; then
								echo Running $executable "data/$n/${lengths[s]}.fq" "data/$n/${lengths[s]}.fq" $n $n
								$executable "data/$n/${lengths[s]}.fq" "data/$n/${lengths[s]}.fq" $n $n
							fi
							time_end=$(date +%s)
							time=$(awk '{ sum += $1; n++ } END { if (n > 0) print sum / n; else print 0 }' times.txt)
							energy=$(awk '{ sum += $1; n++ } END { if (n > 0) print sum / n; else print 0 }' energy.txt)
							echo "${n},${lengths[s]},$time,$energy,$acc,$nth,$time_start,$time_end" >> "$benchmark"
								[ -f "Scores.bin" ] && md5sum "Scores.bin" > golden/acc${acc}_${n}_${lengths[s]}_${nth}.md5 && rm "Scores.bin"
							rm times.txt
							rm energy.txt
						else
							echo "File data/$n/${lengths[s]}.fq not found"
						fi
					#else
					#	break
					#fi
				done
			done
		done
	  if [[ "$executable" == "./SW_fpga/seqmatcher" ]]; then
		  cp benchmark.csv benchmarks/acc${acc}_${NUM_WORKERS}w${TARGET_FREQ_MHZ}MHz.csv
      if [[ -f "benchmarks.csv" ]]; then
        rm benchmark.csv
      fi
	  fi
	done

	if [[ "$executable" == "./SW_fpga/seqmatcher" ]]; then
		cd SW_fpga/driver
        	./unload
		cd ../..
        fi
fi
