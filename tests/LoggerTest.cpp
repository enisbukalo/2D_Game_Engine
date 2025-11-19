#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include "utility/Logger.h"

class LoggerTest : public ::testing::Test
{
protected:
    const std::string TEST_LOG_FILE = "test_logger.log";

    void SetUp() override
    {
        // Clean up any existing test log file
        if (std::filesystem::exists(TEST_LOG_FILE))
        {
            std::filesystem::remove(TEST_LOG_FILE);
        }
    }

    void TearDown() override
    {
        // Shutdown logger to flush any pending messages
        Logger::instance().shutdown();

        // Clean up test log file
        if (std::filesystem::exists(TEST_LOG_FILE))
        {
            std::filesystem::remove(TEST_LOG_FILE);
        }
    }

    std::string readLogFile()
    {
        std::ifstream file(TEST_LOG_FILE);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool logFileContains(const std::string& text)
    {
        std::string content = readLogFile();
        return content.find(text) != std::string::npos;
    }

    int countOccurrences(const std::string& text)
    {
        std::string content = readLogFile();
        int count = 0;
        size_t pos = 0;
        while ((pos = content.find(text, pos)) != std::string::npos)
        {
            ++count;
            pos += text.length();
        }
        return count;
    }
};

TEST_F(LoggerTest, Initialization)
{
    // Initialize logger
    Logger::instance().init(TEST_LOG_FILE);

    // Log a simple message
    LOG_INFO("Test initialization");

    // Give logger thread time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Shutdown to flush
    Logger::instance().shutdown();

    // Check if file exists
    EXPECT_TRUE(std::filesystem::exists(TEST_LOG_FILE));

    // Check if message was logged
    EXPECT_TRUE(logFileContains("Test initialization"));
}

TEST_F(LoggerTest, MultipleInitializationsCalls)
{
    // Initialize logger multiple times (should be safe due to guard)
    Logger::instance().init(TEST_LOG_FILE);
    Logger::instance().init(TEST_LOG_FILE);
    Logger::instance().init(TEST_LOG_FILE);

    LOG_INFO("Multiple init test");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    EXPECT_TRUE(logFileContains("Multiple init test"));
}

TEST_F(LoggerTest, DebugLevel)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_DEBUG("Debug message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Debug message") != std::string::npos);
    EXPECT_TRUE(content.find("[DEBUG") != std::string::npos);
}

TEST_F(LoggerTest, InfoLevel)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_INFO("Info message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Info message") != std::string::npos);
    EXPECT_TRUE(content.find("[INFO") != std::string::npos);
}

TEST_F(LoggerTest, WarningLevel)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_WARNING("Warning message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("[WARNING") != std::string::npos);
}

TEST_F(LoggerTest, ErrorLevel)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_ERROR("Error message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("[ERROR") != std::string::npos);
}

TEST_F(LoggerTest, StreamStyleLogging)
{
    Logger::instance().init(TEST_LOG_FILE);

    int value = 42;
    float pi = 3.14159f;

    LOG_INFO_STREAM("Integer: " << value << ", Float: " << pi);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Integer: 42") != std::string::npos);
    EXPECT_TRUE(content.find("Float: 3.14159") != std::string::npos);
}

TEST_F(LoggerTest, MultipleMessages)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_DEBUG("First message");
    LOG_INFO("Second message");
    LOG_WARNING("Third message");
    LOG_ERROR("Fourth message");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_TRUE(content.find("First message") != std::string::npos);
    EXPECT_TRUE(content.find("Second message") != std::string::npos);
    EXPECT_TRUE(content.find("Third message") != std::string::npos);
    EXPECT_TRUE(content.find("Fourth message") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelFiltering)
{
    Logger::instance().init(TEST_LOG_FILE);

    // Set minimum log level to WARNING
    Logger::instance().setLogLevel(LogLevel::WARNING);

    LOG_DEBUG("Debug - should not appear");
    LOG_INFO("Info - should not appear");
    LOG_WARNING("Warning - should appear");
    LOG_ERROR("Error - should appear");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    EXPECT_FALSE(content.find("Debug - should not appear") != std::string::npos);
    EXPECT_FALSE(content.find("Info - should not appear") != std::string::npos);
    EXPECT_TRUE(content.find("Warning - should appear") != std::string::npos);
    EXPECT_TRUE(content.find("Error - should appear") != std::string::npos);
}

TEST_F(LoggerTest, TimestampPresent)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_INFO("Timestamp test");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    // Check for timestamp format: [YYYY-MM-DD HH:MM:SS.mmm]
    EXPECT_TRUE(content.find("[20") != std::string::npos);  // Year starts with 20
    EXPECT_TRUE(content.find(".") != std::string::npos);    // Milliseconds separator
}

TEST_F(LoggerTest, ThreadIdPresent)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_INFO("Thread ID test");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::instance().shutdown();

    std::string content = readLogFile();
    // Check for thread ID format: [Thread:...]
    EXPECT_TRUE(content.find("[Thread:") != std::string::npos);
}

TEST_F(LoggerTest, AsyncLogging)
{
    Logger::instance().init(TEST_LOG_FILE);

    // Log many messages quickly to test async behavior
    for (int i = 0; i < 100; ++i)
    {
        LOG_INFO_STREAM("Message " << i);
    }

    // Give logger time to process all messages
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Logger::instance().shutdown();

    // Count occurrences of "Message"
    int count = countOccurrences("Message");
    EXPECT_EQ(count, 100);
}

TEST_F(LoggerTest, MultiThreadedLogging)
{
    Logger::instance().init(TEST_LOG_FILE);

    // Create multiple threads logging simultaneously
    auto logFunc = [](int threadNum) {
        for (int i = 0; i < 10; ++i)
        {
            LOG_INFO_STREAM("Thread " << threadNum << " message " << i);
        }
    };

    std::thread t1(logFunc, 1);
    std::thread t2(logFunc, 2);
    std::thread t3(logFunc, 3);

    t1.join();
    t2.join();
    t3.join();

    // Give logger time to process all messages
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Logger::instance().shutdown();

    // Check that all messages were logged (3 threads * 10 messages each = 30)
    int count = countOccurrences("message");
    EXPECT_EQ(count, 30);
}

TEST_F(LoggerTest, ShutdownFlushesAllMessages)
{
    Logger::instance().init(TEST_LOG_FILE);

    // Log a message
    LOG_INFO("Final message");

    // Immediately shutdown (should flush pending message)
    Logger::instance().shutdown();

    // Check that message was written
    EXPECT_TRUE(logFileContains("Final message"));
}

TEST_F(LoggerTest, MultipleShutdownCalls)
{
    Logger::instance().init(TEST_LOG_FILE);

    LOG_INFO("Shutdown test");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Multiple shutdown calls should be safe
    Logger::instance().shutdown();
    Logger::instance().shutdown();
    Logger::instance().shutdown();

    EXPECT_TRUE(logFileContains("Shutdown test"));
}
