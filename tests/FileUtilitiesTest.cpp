#include <gtest/gtest.h>
#include <filesystem>
#include "FileUtilities.h"

class FileUtilitiesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test directory if it doesn't exist.
        if (!std::filesystem::exists("tests/test_data"))
        {
            std::filesystem::create_directory("tests/test_data");
        }
    }

    void TearDown() override
    {
        // Clean up test files
        if (std::filesystem::exists("tests/test_data/test_write.txt"))
        {
            std::filesystem::remove("tests/test_data/test_write.txt");
        }
    }
};

TEST_F(FileUtilitiesTest, WriteAndReadFile)
{
    const std::string testPath = "tests/test_data/test_write.txt";
    const std::string content  = "Hello, World!\nThis is a test file.";

    // Test writing
    ASSERT_NO_THROW(FileUtilities::writeFile(testPath, content));

    // Test reading
    std::string readContent;
    ASSERT_NO_THROW(readContent = FileUtilities::readFile(testPath));
    EXPECT_EQ(readContent, content);
}

TEST_F(FileUtilitiesTest, ReadNonExistentFile)
{
    EXPECT_THROW(FileUtilities::readFile("nonexistent.txt"), std::runtime_error);
}

TEST_F(FileUtilitiesTest, WriteToInvalidPath)
{
    EXPECT_THROW(FileUtilities::writeFile("/invalid/path/file.txt", "content"), std::runtime_error);
}

TEST_F(FileUtilitiesTest, EmptyFileOperations)
{
    const std::string testPath     = "tests/test_data/test_write.txt";
    const std::string emptyContent = "";

    // Test writing empty content
    ASSERT_NO_THROW(FileUtilities::writeFile(testPath, emptyContent));

    // Test reading empty file
    std::string readContent;
    ASSERT_NO_THROW(readContent = FileUtilities::readFile(testPath));
    EXPECT_EQ(readContent, emptyContent);
}