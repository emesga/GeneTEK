MACHINE_ARCH := $(shell uname -m)
ifeq ($(MACHINE_ARCH), armv7l)
	DEFINES = -DIS_PYNQZ2_ARCH
else ifeq ($(MACHINE_ARCH), aarch64)
	DEFINES = -DIS_ZCU104_ARCH
else
	$(error Unsupported machine architecture: $(MACHINE_ARCH))
endif

all: programOverlay

programOverlay: programOverlay.cpp clock.* bitloader.* parser.hpp
	g++ -O3 -g programOverlay.cpp clock.cpp bitloader.cpp -I./ $(DEFINES) -o programOverlay -lm -lcma

clean:
	rm -f programOverlay
