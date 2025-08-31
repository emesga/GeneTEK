all: seqmatcher bitloader driver

seqmatcher: src/HW_split_block.cpp src/util.* src/CAccelDriver.* src/CSeqMatcher.*
	g++ -O3 -g src/HW_split_block.cpp src/util.cpp src/CAccelDriver.cpp src/CSeqMatcher.cpp -Ipmt-lib/include/pmt/common -Ipmt-lib/include/pmt -Ipmt-lib/include -I./src/ -o seqmatcher -lm -lcma -lpthread -lpmt

bitloader:
	make -C bitloader

driver:
	make -C driver

clean:
	rm -f seqmatcher
	make -C bitloader clean
	cd driver && ./clean && cd ..
