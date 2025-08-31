#ifndef PMT_CONFIG_H_
#define PMT_CONFIG_H_

/* #undef PMT_BUILD_POWERSENSOR2 */
/* #undef PMT_BUILD_POWERSENSOR3 */
/* #undef PMT_BUILD_NVML */
/* #undef PMT_BUILD_ROCM */
/* #undef PMT_BUILD_RAPL */
/* #undef PMT_BUILD_TEGRA */
#define PMT_BUILD_XILINX
/* #undef PMT_BUILD_LIKWID */
/* #undef PMT_BUILD_CRAY */
/* #undef PMT_BUILD_NVIDIA */

#include <pmt/Dummy.h>
#if defined(PMT_BUILD_POWERSENSOR2)
#include <pmt/PowerSensor2.h>
#endif
#if defined(PMT_BUILD_POWERSENSOR3)
#include <pmt/PowerSensor3.h>
#endif
#if defined(PMT_BUILD_NVML)
#include <pmt/NVML.h>
#endif
#if defined(PMT_BUILD_ROCM)
#include <pmt/ROCM.h>
#endif
#if defined(PMT_BUILD_RAPL)
#include <pmt/Rapl.h>
#endif
#if defined(PMT_BUILD_TEGRA)
#include <pmt/Tegra.h>
#endif
#if defined(PMT_BUILD_XILINX)
#include <pmt/Xilinx.h>
#endif
#if defined(PMT_BUILD_LIKWID)
#include <pmt/Likwid.h>
#endif
#if defined(PMT_BUILD_CRAY)
#include <pmt/Cray.h>
#endif
#if defined(PMT_BUILD_NVIDIA)
#include <pmt/NVIDIA.h>
#endif

#endif
