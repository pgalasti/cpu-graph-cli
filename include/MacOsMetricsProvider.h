#ifndef MACOS_METRICS_PROVIDER_H
#define MACOS_METRICS_PROVIDER_H

#include <vector>
#include <mach/mach.h>
#include "IMetricsProvider.h"

namespace CpuCli {

  class MacOsMetricsProvider : public IMetricsProvider {
  public:
    MacOsMetricsProvider();
    ~MacOsMetricsProvider() override = default;

    CpuMetrics Parse() override;
    const CpuInfo& getCpuInfo() const override { return m_cpuInfo; }

  private:
    struct CpuTick {
      integer_t user   = 0;
      integer_t system = 0;
      integer_t idle   = 0;
      integer_t nice   = 0;

      unsigned long long total() const {
        return static_cast<unsigned long long>(user) + system + idle + nice;
      }
    };

    std::vector<CpuTick> m_prevTicks;
    bool m_firstRead = true;
    CpuInfo m_cpuInfo;

    std::vector<CpuTick> readCpuTicks() const;
    MemoryMetrics readMemory() const;
    CpuInfo readCpuInfo() const;
  };

} // namespace CpuCli

#endif // MACOS_METRICS_PROVIDER_H
