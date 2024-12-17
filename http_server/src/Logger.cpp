#include "Logger.h"

#include <bits/types/struct_timeval.h>
#include <sys/time.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <ratio>
#include <sstream>
#include <string>
#include <string_view>

namespace log {

static constexpr unsigned int LEVEL_FMT_WIDTH = 8;
static constexpr unsigned int TIME_FMT_WIDTH  = 25;
static constexpr unsigned int FULL_LOG_WIDTH  = 100;

Manager manager;

void addConsole(Level level) { manager.addWorker("", level); }
void removeConsole() { manager.removeWorker(""); }
void addFile(const std::filesystem::path& path, Level level) { manager.addWorker(path, level); }
void removeFile(const std::filesystem::path& path) { manager.removeWorker(path); }

namespace {
std::string timeStr() {
  timeval time_value{};
  gettimeofday(&time_value, nullptr);
  tm time_struct{};
  localtime_r(&time_value.tv_sec, &time_struct);
  std::ostringstream output_stream;
  output_stream << std::put_time(&time_struct, "%F %T") << "." << std::setw(3) << std::setfill('0')
                << time_value.tv_usec / std::milli::den;
  return output_stream.str();
}

std::string_view colorStr(Level level) {
  switch (level) {
    case CRITICAL: return "\033[31;1m";
    case ERROR:    return "\033[31m";
    case WARNING:  return "\033[33m";
    case INFO:     return "";
    case DEBUG:    return "\033[2m";
    case TRACE:    return "\033[2;3m";
  }
  return "";
}

std::string_view levelStr(Level level) {
  switch (level) {
    case CRITICAL: return "CRITICAL";
    case ERROR:    return "ERROR";
    case WARNING:  return "WARNING";
    case INFO:     return "INFO";
    case DEBUG:    return "DEBUG";
    case TRACE:    return "TRACE";
  }
  return "";
}

constexpr std::string_view resetColor() { return "\033[0m"; }
}    // namespace

Worker::Worker(const std::filesystem::path& path, Level level)
    : m_path(path)
    , m_level(level)
    , m_stream(nullptr) {
  if (m_path.empty()) { return; }
  if (!std::filesystem::exists(path) && path.has_parent_path()) {
    std::filesystem::create_directories(path.parent_path());
  }
  m_stream = std::make_unique<std::ofstream>(path, std::ios::app);
  if (!good()) {
    LOG(WARNING) << "Failed to open log at " << (console() ? "console" : path);
  } else {
    (*m_stream) << "\n"
                << std::string(TIME_FMT_WIDTH, '-') << "\n"
                << timeStr() << " | " << std::string(FULL_LOG_WIDTH - TIME_FMT_WIDTH - 1, '-')
                << "\n";
  }
}

Manager::Init::Init() {
  if (m_ref_count++ == 0) { new (&manager) Manager(); }
}

Manager::Init::~Init() {
  if (--m_ref_count == 0) { manager.~Manager(); }
}

void Manager::addWorker(const std::filesystem::path& path, Level level) {
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    for (Worker& worker : m_workers) {
      if (worker.path() == path) {
        worker.setLevel(level);
        return;
      }
    }
    const Worker& new_worker = m_workers.emplace_back(path, level);
    if (!new_worker.good()) { m_workers.pop_back(); }
  }
  LOG(DEBUG) << "Added log worker at " << (path.empty() ? "console" : path)
            << " (level: " << levelStr(level) << ")";
}

void Manager::removeWorker(const std::filesystem::path& path) {
  LOG(DEBUG) << "Removing log worker at " << (path.empty() ? "console" : path);
  const std::lock_guard<std::mutex> lock(m_mutex);
  for (auto itr = m_workers.begin(); itr != m_workers.end(); ++itr) {
    if (itr->path() == path) {
      m_workers.erase(itr);
      break;
    }
  }
}

void Manager::log(const Line& line) {
  const std::lock_guard<std::mutex> lock(m_mutex);
  for (Worker& worker : m_workers) { worker.log(line); }
}

void Worker::log(const Line& line) {
  if (!good() || line.level() > m_level) { return; }
  if (console()) { stream() << colorStr(line.level()); }
  stream() << timeStr() << " | [" << std::setw(LEVEL_FMT_WIDTH) << levelStr(line.level()) << "] "
           << line.file() << ":" << line.line() << " in " << line.func() << "(): ";
  std::string msg = line.msg();
  while (msg.back() == '\n') { msg.pop_back(); }
  std::size_t start = 0;
  while (start < msg.size()) {
    std::size_t end = msg.find('\n', start);
    if (end == std::string::npos) { end = msg.size(); }
    if (start > 0) { stream() << std::string(TIME_FMT_WIDTH + LEVEL_FMT_WIDTH + 3, ' ') << " -> "; }
    stream() << msg.substr(start, end - start + 1);
    start = end + 1;
  }
  if (console()) { stream() << resetColor(); }
  stream() << "\n";
}

}    // namespace log
