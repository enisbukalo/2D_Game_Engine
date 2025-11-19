#include "utility/Logger.h"
#include <iomanip>
#include <iostream>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger() : m_running(false), m_minLogLevel(LogLevel::DEBUG), m_initialized(false)
{
}

Logger::~Logger()
{
    shutdown();
}

void Logger::init(const std::string& logFilePath)
{
    if (m_initialized)
    {
        return;
    }

    m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!m_logFile.is_open())
    {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        return;
    }

    m_running = true;
    m_workerThread = std::thread(&Logger::workerThread, this);
    m_initialized = true;
}

void Logger::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_running = false;
    m_condition.notify_one();

    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }

    m_initialized = false;
}

void Logger::debug(const std::string& message)
{
    if (m_minLogLevel <= LogLevel::DEBUG)
    {
        enqueue(LogEntry(LogLevel::DEBUG, message));
    }
}

void Logger::info(const std::string& message)
{
    if (m_minLogLevel <= LogLevel::INFO)
    {
        enqueue(LogEntry(LogLevel::INFO, message));
    }
}

void Logger::warning(const std::string& message)
{
    if (m_minLogLevel <= LogLevel::WARNING)
    {
        enqueue(LogEntry(LogLevel::WARNING, message));
    }
}

void Logger::error(const std::string& message)
{
    if (m_minLogLevel <= LogLevel::ERROR)
    {
        enqueue(LogEntry(LogLevel::ERROR, message));
    }
}

void Logger::setLogLevel(LogLevel level)
{
    m_minLogLevel = level;
}

void Logger::enqueue(const LogEntry& entry)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_logQueue.push(entry);
    }
    m_condition.notify_one();
}

void Logger::workerThread()
{
    while (m_running)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_condition.wait(lock, [this] { return !m_logQueue.empty() || !m_running; });

        while (!m_logQueue.empty())
        {
            LogEntry entry = m_logQueue.front();
            m_logQueue.pop();
            lock.unlock();

            std::string formattedMessage = formatLogEntry(entry);

            // Write to console
            std::cout << formattedMessage << std::endl;

            // Write to file
            if (m_logFile.is_open())
            {
                m_logFile << formattedMessage << std::endl;
                m_logFile.flush();
            }

            lock.lock();
        }
    }

    // Flush remaining messages
    while (!m_logQueue.empty())
    {
        LogEntry entry = m_logQueue.front();
        m_logQueue.pop();

        std::string formattedMessage = formatLogEntry(entry);
        std::cout << formattedMessage << std::endl;

        if (m_logFile.is_open())
        {
            m_logFile << formattedMessage << std::endl;
        }
    }
}

std::string Logger::formatLogEntry(const LogEntry& entry) const
{
    // Convert timestamp to string
    auto time_t_now = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(entry.timestamp.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream oss;
    oss << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count()
        << "] ";

    // Add log level
    oss << "[" << std::setw(7) << std::left << logLevelToString(entry.level) << "] ";

    // Add thread ID
    oss << "[Thread:" << entry.threadId << "] ";

    // Add message
    oss << entry.message;

    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
