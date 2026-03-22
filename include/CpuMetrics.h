#ifndef CPU_METRICS_H
#define CPU_METRICS_H

#include <string>
#include <vector>

namespace CpuCli {

  struct CpuInfo {
    std::string modelName;
    std::string vendorId;
    unsigned int physicalCores = 0;
    unsigned int logicalCores  = 0;
  };

  struct MemoryMetrics {
    unsigned long totalKB     = 0;
    unsigned long freeKB      = 0;
    unsigned long availableKB = 0;
    unsigned long usedKB      = 0;
  };

  struct CpuMetrics {
    std::vector<unsigned short> cpuUtilization; // 0-100 per logical CPU
    MemoryMetrics memory;
  };

} // namespace CpuCli

#endif // CPU_METRICS_H
