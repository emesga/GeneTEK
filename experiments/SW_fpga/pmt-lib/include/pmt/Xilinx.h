#ifndef PMT_XILINX_H_
#define PMT_XILINX_H_

#include "common/PMT.h"

#include <memory>
#include <string>

namespace pmt::xilinx {
class Xilinx : public PMT {
 public:
  inline static std::string name = "xilinx";
  static std::unique_ptr<Xilinx> Create(
      const char *device = default_device().c_str());

  /**
   * LIST OF KNOWN DEVICES
  */

  static std::string default_device() {
    return "/sys/devices/pci0000:a0/0000:a0:03.1/0000:a1:00.0/hwmon/hwmon3/"
           "power1_input";
  }

  /**
   * Ultrascale+ (ZCU104)
  */

  /**
   * Measures most of the FPGA chip
  */
  static std::string ultrascale_ZCU104() {
    return "/sys/bus/i2c/drivers/pmbus/4-0043/hwmon/hwmon0/"
           "power1_input";
  }
  /**
   * Measures PS (Full Power and Low Power domains) + PL
   * PS-FP: 4 CPUs + caches + DDR mem controller + GPUs
   * PS-LP: OCM (scratchpad), 2xARM cores, 2xTCM memoties and 2 USBs
   * PL: BRAMs, DSPs, LUTs, IOs, VCU
  */
  static std::string ultrascale_ZCU104_PSPL() {
    return "/sys/bus/i2c/drivers/pmbus/4-0043/hwmon/hwmon0/"
           "power2_input";
  }
  /**
   * Measures the IO (MIO 0-2)
  */
  static std::string ultrascale_ZCU104_IO() {
    return "/sys/bus/i2c/drivers/pmbus/4-0043/hwmon/hwmon0/"
           "power3_input";
  }
  /**
   * Meausures all DDRs (4 x PS_DDR and PL_DDR(SO-DIMM))
  */
  static std::string ultrascale_ZCU104_DDR() {
    return "/sys/bus/i2c/drivers/pmbus/4-0043/hwmon/hwmon0/"
           "power4_input";
  }
};
}  // end namespace pmt::xilinx

#endif  // PMT_XILINX_H_
