#ifndef I_METRICS_PROVIDER_H
#define I_METRICS_PROVIDER_H

#include "CpuMetrics.h"

namespace CpuCli {

  class IMetricsProvider {
  public:
    virtual ~IMetricsProvider() = default;

    virtual CpuMetrics Parse() = 0;
    virtual const CpuInfo& getCpuInfo() const = 0;
  };

} // namespace CpuCli

#endif // I_METRICS_PROVIDER_H
