#include "MacOsMetricsProvider.h"

#include <sys/sysctl.h>
#include <mach/mach.h>

using namespace CpuCli;

MacOsMetricsProvider::MacOsMetricsProvider() {
  m_cpuInfo = readCpuInfo();
}

CpuInfo MacOsMetricsProvider::readCpuInfo() const {

  CpuInfo info;
  size_t size;

  char brand[256] = {};
  size = sizeof(brand);
  sysctlbyname("machdep.cpu.brand_string", brand, &size, nullptr, 0);
  info.modelName = brand;

  char vendor[256] = {};
  size = sizeof(vendor);
  sysctlbyname("machdep.cpu.vendor", vendor, &size, nullptr, 0);
  info.vendorId = vendor;

  int physCores = 0;
  size = sizeof(physCores);
  sysctlbyname("hw.physicalcpu", &physCores, &size, nullptr, 0);
  info.physicalCores = static_cast<unsigned int>(physCores);

  int logCores = 0;
  size = sizeof(logCores);
  sysctlbyname("hw.logicalcpu", &logCores, &size, nullptr, 0);
  info.logicalCores = static_cast<unsigned int>(logCores);

  return info;
}

MemoryMetrics MacOsMetricsProvider::readMemory() const {

  MemoryMetrics mem;

  uint64_t totalBytes = 0;
  size_t size = sizeof(totalBytes);
  sysctlbyname("hw.memsize", &totalBytes, &size, nullptr, 0);
  mem.totalKB = totalBytes / 1024;

  vm_size_t pageSize = 0;
  host_page_size(mach_host_self(), &pageSize);

  vm_statistics64_data_t vmStats = {};
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  host_statistics64(mach_host_self(), HOST_VM_INFO64,
                    reinterpret_cast<host_info64_t>(&vmStats), &count);

  mem.freeKB      = (static_cast<uint64_t>(vmStats.free_count)     * pageSize) / 1024;
  mem.availableKB = (static_cast<uint64_t>(vmStats.free_count +
                                            vmStats.inactive_count) * pageSize) / 1024;
  mem.usedKB      = mem.totalKB - mem.freeKB;

  return mem;
}

std::vector<MacOsMetricsProvider::CpuTick> MacOsMetricsProvider::readCpuTicks() const {

  std::vector<CpuTick> ticks;

  processor_info_array_t cpuInfo = nullptr;
  mach_msg_type_number_t numCpuInfo = 0;
  natural_t numCpus = 0;

  kern_return_t kr = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO,
                                          &numCpus, &cpuInfo, &numCpuInfo);
  if (kr != KERN_SUCCESS) return ticks;

  ticks.resize(numCpus);
  for (natural_t i = 0; i < numCpus; ++i) {
    ticks[i].user   = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_USER];
    ticks[i].system = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_SYSTEM];
    ticks[i].idle   = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_IDLE];
    ticks[i].nice   = cpuInfo[CPU_STATE_MAX * i + CPU_STATE_NICE];
  }

  vm_deallocate(mach_task_self(),
                reinterpret_cast<vm_address_t>(cpuInfo),
                sizeof(integer_t) * numCpuInfo);
  return ticks;
}

CpuMetrics MacOsMetricsProvider::Parse() {

  CpuMetrics metrics;
  metrics.memory = readMemory();

  auto currentTicks = readCpuTicks();
  const size_t numCpus = currentTicks.size();
  metrics.cpuUtilization.resize(numCpus, 0);

  if (!m_firstRead && m_prevTicks.size() == numCpus) {
    for (size_t i = 0; i < numCpus; ++i) {
      unsigned long long deltaTotal  = currentTicks[i].total()             - m_prevTicks[i].total();
      unsigned long long deltaIdle   = static_cast<unsigned long long>(currentTicks[i].idle)
                                     - static_cast<unsigned long long>(m_prevTicks[i].idle);
      unsigned long long deltaActive = deltaTotal - deltaIdle;
      if (deltaTotal > 0) {
        metrics.cpuUtilization[i] = static_cast<unsigned short>(deltaActive * 100 / deltaTotal);
      }
    }
  }

  m_prevTicks = currentTicks;
  m_firstRead = false;
  return metrics;
}
