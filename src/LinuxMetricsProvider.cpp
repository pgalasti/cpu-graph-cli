#include "LinuxMetricsProvider.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <set>

#include <g-lib/util/StringParser.h>

using namespace CpuCli;

LinuxMetricsProvider::LinuxMetricsProvider() {
  m_cpuInfo = readCpuInfo();
}

CpuInfo LinuxMetricsProvider::readCpuInfo() const {

  CpuInfo info;
  GLib::Util::StringParser parser;
  std::ifstream file("/proc/cpuinfo");
  std::string line;

  std::set<std::string> physicalIds;
  unsigned int cpuCoresPerSocket = 0;

  auto trim = [](const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end   = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
  };

  while (std::getline(file, line)) {
    parser.Parse(line, ":");
    if (parser.getSize() < 2) continue;

    parser.getFirst();
    const std::string key   = trim(parser.getToken());
    parser++;
    const std::string value = trim(parser.getToken());

    if      (key == "model name"  && info.modelName.empty())   info.modelName = value;
    else if (key == "vendor_id"   && info.vendorId.empty())    info.vendorId  = value;
    else if (key == "physical id")                             physicalIds.insert(value);
    else if (key == "cpu cores"   && cpuCoresPerSocket == 0)   cpuCoresPerSocket = std::stoul(value);
    else if (key == "processor")                               ++info.logicalCores;
  }

  const unsigned int sockets = physicalIds.empty() ? 1 : static_cast<unsigned int>(physicalIds.size());
  info.physicalCores = cpuCoresPerSocket * sockets;
  return info;
}

std::vector<CpuTick> LinuxMetricsProvider::readCpuTicks() const {

  std::vector<CpuTick> ticks;
  std::ifstream file("/proc/stat");
  std::string line;

  while (std::getline(file, line)) {
    if (line.size() < 4) continue;
    // Only per-CPU lines: cpu0, cpu1, etc. (skip the aggregate "cpu " line)
    if (line.substr(0, 3) != "cpu") continue;
    if (!std::isdigit(static_cast<unsigned char>(line[3]))) continue;

    std::istringstream ss(line);
    std::string label;
    CpuTick tick;
    ss >> label
       >> tick.user >> tick.nice >> tick.system >> tick.idle
       >> tick.iowait >> tick.irq >> tick.softirq >> tick.steal;

    ticks.push_back(tick);
  }

  return ticks;
}

MemoryMetrics LinuxMetricsProvider::readMemory() const {

  MemoryMetrics mem;
  GLib::Util::StringParser parser;
  std::ifstream file("/proc/meminfo");
  std::string line;

  while (std::getline(file, line)) {
    parser.Parse(line, ":");
    if (parser.getSize() < 2) continue;

    parser.getFirst();
    const std::string key = parser.getToken();
    parser++;
    const std::string valueStr = parser.getToken();

    unsigned long value = 0;
    std::istringstream iss(valueStr);
    iss >> value;

    if      (key == "MemTotal")     mem.totalKB     = value;
    else if (key == "MemFree")      mem.freeKB      = value;
    else if (key == "MemAvailable") mem.availableKB = value;
  }

  mem.usedKB = mem.totalKB - mem.freeKB;
  return mem;
}

CpuMetrics LinuxMetricsProvider::Parse() {

  CpuMetrics metrics;
  metrics.memory = readMemory();

  auto currentTicks = readCpuTicks();
  const size_t numCpus = currentTicks.size();
  metrics.cpuUtilization.resize(numCpus, 0);

  if (!m_firstRead && m_prevTicks.size() == numCpus) {
    for (size_t i = 0; i < numCpus; ++i) {
      unsigned long long deltaTotal  = currentTicks[i].total()      - m_prevTicks[i].total();
      unsigned long long deltaIdle   = currentTicks[i].idle_total() - m_prevTicks[i].idle_total();
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
