.PHONY: ip hls_project hls_sim clean cleanall vivado_project bitstream extract_bitstream help
PROJECT_NAME := BlockMatcher
HLS_FOLDER := HLS_v0
HLS_ARGS := --ns=5
EXTRA_VIVADO_EFFORT := 1


help:
	@echo ""
	@echo "MAKEFILE targets"
	@echo ""
	@echo "VITIS HLS targets"
	@echo ""
	@echo "hls_project: Just creates the Vitis HLS project"
	@echo "hls_sim: Creates the Vitis HLS project and runs the C++ simulation"
	@echo "ip: Creates the Vitis HLS project, synthesizes the design and exports the IP core"
	@echo ""
	@echo "VIVADO targets"
	@echo ""
	@echo "vivado_project: Just creates the Vivado project"
	@echo "bitstream: Creates the Vivado project and runs synthesis up to bitstream generation"
	@echo "extract_bitstream: If the Vivado bitstream has already been generated, extracts it to this folder"
	@echo ""
	@echo "Generic targets"
	@echo ""
	@echo "clean: Deletes log files and Vitis HLS and Vivado projects. Deletes the files in the IP Catalog (keeps the IP catalog ZIP file)"
	@echo "cleanall: Additionally, deletes the bitstream files and the IP catalog ZIP file"
	@echo "help: Shows this help message"
	@echo ""


ip: IP-repo/IP-repo.zip $(HLS_FOLDER)/globals.h $(HLS_FOLDER)/logical_block.h $(HLS_FOLDER)/Main.cpp $(HLS_FOLDER)/Seqmatcher.cpp

IP-repo/IP-repo.zip: $(HLS_FOLDER)/globals.h $(HLS_FOLDER)/logical_block.h $(HLS_FOLDER)/Main.cpp $(HLS_FOLDER)/Seqmatcher.cpp
	rm -rf $(PROJECT_NAME)_HLS
	./build_HLS_project.sh $(HLS_ARGS)

hls_project: $(PROJECT_NAME)_HLS/hls.app

$(PROJECT_NAME)_HLS/hls.app:
	rm -rf $(PROJECT_NAME)_HLS
	./build_HLS_project.sh $(HLS_ARGS)

hls_sim:
	rm -rf $(PROJECT_NAME)_HLS
	vitis_hls -f $(PROJECT_NAME)_HLS_sim.tcl -tclargs 0 5

vivado_project: ip $(PROJECT_NAME)_Vivado/

$(PROJECT_NAME)_Vivado/:
	cd IP-repo; unzip ./IP-repo.zip; cd ..
ifdef EXTRA_VIVADO_EFFORT
	@echo "\n\n======\nUSING VIVADO EXTRA TIMING EFFORT SETTINGS\n======"
	cp BlockMatcher_Vivado_extraTimingEffort.tcl BlockMatcher_Vivado.tcl
	@echo "\n\n"
else
	@echo "\n\n======\nUSING VIVADO NORMAL EFFORT SETTINGS\n======"
	cp BlockMatcher_Vivado_NormalEffort.tcl BlockMatcher_Vivado.tcl
	@echo "\n\n"
endif
	@sleep 1
	vivado -mode batch -source $(PROJECT_NAME)_Vivado.tcl

bitstream: vivado_project $(PROJECT_NAME).bit $(PROJECT_NAME).hwh

$(PROJECT_NAME).bit $(PROJECT_NAME).hwh:
	vivado -mode batch -source run_vivado.tcl
	cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.runs/impl_1/design_1_wrapper.bit $(PROJECT_NAME).bit
	cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh $(PROJECT_NAME).hwh
	#cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.runs/impl_1/design_1_wrapper.ltx $(PROJECT_NAME).ltx
	@md5sum $(PROJECT_NAME).bit
	@md5sum $(PROJECT_NAME).hwh


extract_bitstream:
	cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.runs/impl_1/design_1_wrapper.bit $(PROJECT_NAME).bit
	cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh $(PROJECT_NAME).hwh
	#cp ./$(PROJECT_NAME)_Vivado/$(PROJECT_NAME)_Vivado.runs/impl_1/design_1_wrapper.ltx $(PROJECT_NAME).ltx
	@md5sum $(PROJECT_NAME).bit
	@md5sum $(PROJECT_NAME).hwh


clean:
	rm -rf NA/ .Xil
	rm -f vivado*.jou vivado*.log vivado*.str vitis_hls.log
	rm -f $(PROJECT_NAME)_Vivado_def_val.txt $(PROJECT_NAME)_Vivado_dump.txt
	rm -rf $(PROJECT_NAME)_HLS $(PROJECT_NAME)_Vivado
	rm -f IP-repo/component.xml
	rm -rf IP-repo/constraints/ IP-repo/doc/ IP-repo/drivers/ IP-repo/hdl/ IP-repo/misc/ IP-repo/xgui/
	rm -f BlockMatcher_Vivado.tcl

cleanall: clean
	rm -f IP-repo/IP-repo.zip
	rm -f *.bit *.hwh *.ltx
	rm -f design_power_report.txt design_timing_report.txt design_utilization_report.txt

