#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

namespace log {

/**
 * An enum representing the severity of a log messsage
 */
enum Level : std::uint8_t {
  CRITICAL,    // Unrecoverable error, should only precede a crash
  ERROR,       // Recoverable error, not expected during normal execution
  WARNING,     // Something unexpected occurred, but it is not a problem
  INFO,        // General runtime information about what is happening
  DEBUG,       // Detailed runtime information
  TRACE        // Highly detailed runtime information
};

/**
 * Adds `std::cout` to the list of log workers. If already open, updates the log level to `level`
 *
 * @param level The level for console logging
 */
void addConsole(Level level = INFO);

/**
 * Removes `std::cout` from the list of log workers. No effect if not already logging there
 */
void removeConsole();

/**
 * Adds a log file at `path` to the list of log files
 *
 * @param path The location to open the new log file
 * @param level The log level for opening the new file
 */
void addFile(const std::filesystem::path& path, Level level = INFO);

/**
 * Removes the log file at `path` from the list of log workers. No effect if not present
 */
void removeFile(const std::filesystem::path& path);

class Line;

// NOLINTBEGIN (cppcore-guidelines-macro-usage): Need macro for location info
/**
 * A macro for generating a `log::Line` object in its own scope, triggering the line to be logged
 * immediately
 */
#define LOG(level) \
  if (true) log::Line(level, __FILE__, __LINE__, __FUNCTION__)
// NOLINTEND (cppcore-guidelines-macro-usage)

class Worker {
 public:
  Worker(const std::filesystem::path& path, Level level);

  [[nodiscard]] const std::filesystem::path& path() const { return m_path; }
  [[nodiscard]] bool                         console() const { return m_path.empty(); }
  [[nodiscard]] Level                        level() const { return m_level; }
  [[nodiscard]] bool                         good() const {
    return (m_path.empty() && std::cout.good()) || (m_stream && m_stream->good());
  }

  void setLevel(Level level) { m_level = level; }

  void log(const Line& line);

 private:
  [[nodiscard]] std::ostream& stream() { return m_path.empty() ? std::cout : *m_stream; }

  std::filesystem::path          m_path;
  Level                          m_level;
  std::unique_ptr<std::ofstream> m_stream;
};

/**
 * Responsible for managing the logging streams. Can log to `std::cout` as well as multiple files
 */
class Manager {
 public:
  /**
   * Adds a worker at the given `path` and `level`
   *
   * @param path The location to open the file, or `""` for `std::cout`
   * @param level The level to set the worker to
   */
  void addWorker(const std::filesystem::path& path, Level level);

  /**
   * Removes a worker at the given `path`
   *
   * @param path The location to stop logging at
   */
  void removeWorker(const std::filesystem::path& path);

  /**
   * Logs a line to the current log workers
   *
   * @param line The `LogLine` object to print to the stream
   */
  void log(const Line& line);

  /**
   * Manages the initialization of the singleton `Manager` class. Creates an instance upon first
   * instantiation, and removes it upon destruction
   */
  class Init {
   public:
    /**
     * Creates an instance of `Manager` if one has not been created
     */
    Init();

    /**
     * Destroys the instance of `Manager` if this is the last reference to it
     */
    ~Init();

    // Should not be copyable or movable
    Init(const Init&)             = delete;
    Init& operator=(const Init&)  = delete;
    Init(const Init&&)            = delete;
    Init& operator=(const Init&&) = delete;

   private:
    std::atomic<unsigned int> m_ref_count = 0;    // Counts the references to the `Manager`
  };

 private:
  std::mutex          m_mutex;    // Ensures log access is thread-safe and consistent
  std::vector<Worker> m_workers;

  static Init m_init;    // Ensures only one instance exists
};
// NOLINTNEXTLINE (cppcoreguidelines-avoid-non-const-global-variables)
extern Manager manager;    // Globally accessible logger instance

/**
 * An object representing a single log line. Upon destruction, it will use the global `Logger
 * logger` instance to log this line to all output streams
 */
class Line {
 public:
  /**
   * Creates a `LogLine` object, which upon destruction logs the line to all open log streams
   *
   * @param level The level of the log message
   * @param file The file of the log message
   * @param line The line number of the log message
   * @param func The function of the log message
   */
  Line(Level level, const std::filesystem::path& file, int line, std::string_view func)
      : m_level(level)
      , m_file(file.filename().string())
      , m_line(line)
      , m_func(func) {}

  /**
   * Logs the object to the open log streams
   */
  ~Line() { manager.log(*this); }

  // Should not be copyable or movable
  Line(const Line&)             = delete;
  Line& operator=(const Line&)  = delete;
  Line(const Line&&)            = delete;
  Line& operator=(const Line&&) = delete;

  /**
   * Forward arguments to the buffer to be printed
   */
  template <typename T> Line& operator<<(const T& obj) {
    m_stream << obj;
    return *this;
  }

  Level              level() const { return m_level; }
  const std::string& file() const { return m_file; }
  int                line() const { return m_line; }
  const std::string& func() const { return m_func; }
  std::string        msg() const { return m_stream.str(); }

 private:
  Level             m_level;
  std::string       m_file;
  int               m_line;
  std::string       m_func;
  std::stringstream m_stream;
};

}    // namespace log

using enum log::Level;
