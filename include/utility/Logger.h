#ifndef LOGGER_H
#define LOGGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

/**
 * @brief Log levels for filtering messages
 */
enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief A log entry containing level, timestamp, and message
 */
struct LogEntry
{
    LogLevel                                          level;
    std::chrono::system_clock::time_point             timestamp;
    std::string                                       message;
    std::thread::id                                   threadId;

    LogEntry(LogLevel lvl, const std::string& msg)
        : level(lvl), timestamp(std::chrono::system_clock::now()), message(msg), threadId(std::this_thread::get_id())
    {
    }
};

/**
 * @brief Thread-safe asynchronous logger with queue-based architecture
 *
 * This logger runs on a separate thread and consumes log entries from a queue,
 * writing them to both console and a log file with timestamps.
 */
class Logger
{
public:
    /**
     * @brief Gets the singleton instance of the logger
     * @return Reference to the Logger instance
     */
    static Logger& instance();

    // Delete copy constructor and assignment operator
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Initialize the logger with a log file path
     * @param logFilePath Path to the log file
     */
    void init(const std::string& logFilePath = "game.log");

    /**
     * @brief Shutdown the logger and flush all pending messages
     */
    void shutdown();

    /**
     * @brief Log a debug message
     * @param message The message to log
     */
    void debug(const std::string& message);

    /**
     * @brief Log an info message
     * @param message The message to log
     */
    void info(const std::string& message);

    /**
     * @brief Log a warning message
     * @param message The message to log
     */
    void warning(const std::string& message);

    /**
     * @brief Log an error message
     * @param message The message to log
     */
    void error(const std::string& message);

    /**
     * @brief Set the minimum log level to display
     * @param level The minimum log level
     */
    void setLogLevel(LogLevel level);

private:
    Logger();
    ~Logger();

    /**
     * @brief Worker thread function that consumes log entries
     */
    void workerThread();

    /**
     * @brief Enqueue a log entry for processing
     * @param entry The log entry to enqueue
     */
    void enqueue(const LogEntry& entry);

    /**
     * @brief Format a log entry as a string
     * @param entry The log entry to format
     * @return Formatted string with timestamp and level
     */
    std::string formatLogEntry(const LogEntry& entry) const;

    /**
     * @brief Convert log level to string
     * @param level The log level
     * @return String representation of the log level
     */
    std::string logLevelToString(LogLevel level) const;

    std::queue<LogEntry>        m_logQueue;       ///< Queue of pending log entries
    std::mutex                  m_queueMutex;     ///< Mutex protecting the queue
    std::condition_variable     m_condition;      ///< Condition variable for thread signaling
    std::thread                 m_workerThread;   ///< Background thread for writing logs
    std::atomic<bool>           m_running;        ///< Flag indicating if logger is running
    std::ofstream               m_logFile;        ///< Output file stream
    LogLevel                    m_minLogLevel;    ///< Minimum log level to display
    bool                        m_initialized;    ///< Flag indicating if logger is initialized
};

// Convenience macros for logging
#define LOG_DEBUG(msg) Logger::instance().debug(msg)
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_WARNING(msg) Logger::instance().warning(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)

// Stream-style logging
#define LOG_DEBUG_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        Logger::instance().debug(oss.str()); \
    } while(0)

#define LOG_INFO_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        Logger::instance().info(oss.str()); \
    } while(0)

#define LOG_WARNING_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        Logger::instance().warning(oss.str()); \
    } while(0)

#define LOG_ERROR_STREAM(stream) \
    do { \
        std::ostringstream oss; \
        oss << stream; \
        Logger::instance().error(oss.str()); \
    } while(0)

#endif  // LOGGER_H
