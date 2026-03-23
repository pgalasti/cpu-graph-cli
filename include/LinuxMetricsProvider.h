#ifndef LINUX_METRICS_PROVIDER_H
#define LINUX_METRICS_PROVIDER_H

#include <vector>
#include "IMetricsProvider.h"

namespace CpuCli {

  struct CpuTick {
    unsigned long long user    = 0;
    unsigned long long nice    = 0;
    unsigned long long system  = 0;
    unsigned long long idle    = 0;
    unsigned long long iowait  = 0;
    unsigned long long irq     = 0;
    unsigned long long softirq = 0;
    unsigned long long steal   = 0;

    unsigned long long total() const {
      return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    unsigned long long idle_total() const {
      return idle + iowait;
    }
  };

  class LinuxMetricsProvider : public IMetricsProvider {
  public:
    LinuxMetricsProvider();
    ~LinuxMetricsProvider() override = default;

    CpuMetrics Parse() override;
    const CpuInfo& getCpuInfo() const override { return m_cpuInfo; }

  private:
    std::vector<CpuTick> m_prevTicks;
    bool m_firstRead = true;
    CpuInfo m_cpuInfo;

    std::vector<CpuTick> readCpuTicks() const;
    MemoryMetrics readMemory() const;
    CpuInfo readCpuInfo() const;
  };

} // namespace CpuCli

#endif // LINUX_METRICS_PROVIDER_H
