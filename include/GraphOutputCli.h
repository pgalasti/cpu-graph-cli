#ifndef CPU_GRAPH_OUTPUT_CLI_H
#define CPU_GRAPH_OUTPUT_CLI_H

#include <vector>
#include <string>

#include "CpuMetrics.h"

namespace CpuCli {

  class OutputCli {
  public:
    OutputCli();
    ~OutputCli() = default;

    void RecordInterval(const CpuMetrics& metrics);
    void OutputProgress(const CpuMetrics& metrics) const;

  private:
    // Per-CPU ring buffers (kept for potential future use)
    std::vector<std::vector<unsigned short>> m_cpuStates;

    std::string RenderProgressBar(unsigned short utilization) const;
  };

} // namespace CpuCli

#endif // CPU_GRAPH_OUTPUT_CLI_H
