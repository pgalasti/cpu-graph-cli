#include "GraphOutputCli.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include "FontColorCli.h"

using namespace CpuCli;

static const unsigned short PROGRESS_BAR_WIDTH = 60;

OutputCli::OutputCli() {}

void OutputCli::RecordInterval(const CpuMetrics& metrics) {

  const size_t numCpus = metrics.cpuUtilization.size();
  if (m_cpuStates.size() != numCpus) {
    m_cpuStates.resize(numCpus);
  }
}

std::string OutputCli::RenderProgressBar(unsigned short utilization) const {

  std::stringstream ss;
  const unsigned short position = PROGRESS_BAR_WIDTH * utilization / 100;

  for (unsigned short i = 0; i < PROGRESS_BAR_WIDTH; ++i) {
    if (i < position) {
      if (utilization > 75)       ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::BackgroundRed);
      else if (utilization > 30)  ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::BackgroundYel);
      else                        ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::BackgroundGre);
    } else {
      ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::BackgroundDft);
    }
    ss << " ";
  }

  ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::BackgroundDft);
  return ss.str();
}

void OutputCli::OutputProgress(const CpuMetrics& metrics) const {

  std::stringstream ss;
  ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Bold);
  ss << "=== CPU Utilization ===" << std::endl;
  ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default);

  for (size_t i = 0; i < metrics.cpuUtilization.size(); ++i) {
    const unsigned short util = metrics.cpuUtilization[i];

    ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Cyan);
    ss << "CPU " << std::setw(2) << i;
    ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::White);
    ss << " [";
    ss << RenderProgressBar(util);
    ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::White);
    ss << "] ";

    if (util > 75)       ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Red);
    else if (util > 30)  ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Yellow);
    else                 ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Green);
    ss << std::setw(3) << util << "%";
    ss << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default);
    ss << std::endl;
  }

  std::cout << ss.str();
}
