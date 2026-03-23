#include <iostream>
#include <thread>
#include <atomic>
#include <iomanip>

#include <unistd.h>
#include <termios.h>

#include <g-lib/util/Stopwatch.h>

#ifdef __APPLE__
  #include "MacOsMetricsProvider.h"
  using MetricsProvider = CpuCli::MacOsMetricsProvider;
#else
  #include "LinuxMetricsProvider.h"
  using MetricsProvider = CpuCli::LinuxMetricsProvider;
#endif

#include "CpuMetrics.h"
#include "GraphOutputCli.h"
#include "FontColorCli.h"

const char CLEAR_SCREEN[] = "\033[2J\033[H";
const double KB_PER_GB = 1024.0 * 1024.0; // 1 GB = 1,048,576 KB

void displayCpuInfo(const CpuCli::CpuInfo& info);
void displayHeader(const CpuCli::MemoryMetrics& mem);
void displayProgressMetrics();
bool stopThread(std::atomic_bool& stop, std::thread& theThread);
char captureInput();

std::atomic_bool progressThread = false;

CpuCli::OutputCli cliOutput;

int main(int argc, char** argv) {

  progressThread = true;
  std::thread currentDisplayThread(displayProgressMetrics);

  char userInput;
  do {
    userInput = captureInput();
  } while (userInput != 'q');

  stopThread(progressThread, currentDisplayThread);
  return 0;
}

void displayCpuInfo(const CpuCli::CpuInfo& info) {

  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Bold);
  std::cout << "=== CPU ===" << std::endl;
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Cyan);
  std::cout << info.modelName << std::endl;
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default);
  std::cout << "Vendor: " << info.vendorId;
  std::cout << "  |  Physical cores: " << info.physicalCores;
  std::cout << "  |  Logical cores: "  << info.logicalCores << std::endl << std::endl;
}

void displayHeader(const CpuCli::MemoryMetrics& mem) {

  const double totalGB = mem.totalKB     / KB_PER_GB;
  const double usedGB  = mem.usedKB      / KB_PER_GB;
  const double freeGB  = mem.freeKB      / KB_PER_GB;
  const double availGB = mem.availableKB / KB_PER_GB;
  const unsigned short usedPct = mem.totalKB > 0
      ? static_cast<unsigned short>(mem.usedKB * 100 / mem.totalKB) : 0;

  std::cout << std::fixed << std::setprecision(1);
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Bold);
  std::cout << "=== System Memory ===" << std::endl;
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Cyan);
  std::cout << "Total: " << totalGB << " GB";
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default) << "  |  ";

  if (usedPct > 75)       std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Red);
  else if (usedPct > 50)  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Yellow);
  else                    std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Green);
  std::cout << "Used: " << usedGB << " GB (" << usedPct << "%)";

  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default);
  std::cout << "  |  Free: " << freeGB << " GB";
  std::cout << "  |  Available: " << availGB << " GB" << std::endl;
  std::cout << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default);
  std::cout << std::endl;
}

void displayProgressMetrics() {

  MetricsProvider parser;
  GLib::Util::Stopwatch sw("progress");

  for (;;) {
    if (!progressThread) return;

    auto metrics = parser.Parse();
    cliOutput.RecordInterval(metrics);

    std::cout << CLEAR_SCREEN;
    displayCpuInfo(parser.getCpuInfo());
    displayHeader(metrics.memory);
    cliOutput.OutputProgress(metrics);
    std::cout << std::endl
              << "["  << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Red)
              << " Q " << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default)
              << "] - " << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Red)
              << "Quit" << CpuCli::Font::ColorOutput(CpuCli::Font::Color::Default)
              << std::endl;

    auto elapsed_ms = sw.Current<std::chrono::milliseconds>();
    if (1000 - elapsed_ms > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 - elapsed_ms));
    }
    sw.Reset();
  }
}

char captureInput() {

  char buf = 0;
  struct termios old = {0};
  fflush(stdout);
  if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN]  = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0) perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
  return buf;
}

bool stopThread(std::atomic_bool& stop, std::thread& theThread) {

  if (!stop) return false;
  stop = false;
  theThread.join();
  return true;
}
