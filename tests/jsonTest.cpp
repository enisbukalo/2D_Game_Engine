#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include "JsonBuilder.h"
#include "JsonParser.h"
#include "JsonValue.h"
#include "TestUtils.h"

TEST(JsonTest, ParseEntityFile)
{
    // Read the entity.json test file
    std::string json = readFile("tests/test_data/entity.json");
    JsonValue   root(json);

    // Test basic entity properties
    EXPECT_EQ(root["id"].getNumber(), 1);
    EXPECT_EQ(root["tag"].getString(), "player");

    // Test components array
    const auto& components = root["components"].getArray();
    ASSERT_EQ(components.size(), 3);

    // Test Transform component
    const auto& transform = components[0];
    EXPECT_EQ(transform["type"].getString(), "Transform");
    const auto& pos = transform["data"]["position"];
    EXPECT_TRUE(approxEqual(pos["x"].getNumber(), 100.0));
    EXPECT_TRUE(approxEqual(pos["y"].getNumber(), 200.0));

    // Test Name component
    const auto& name = components[1];
    EXPECT_EQ(name["type"].getString(), "Name");
    EXPECT_EQ(name["data"]["name"].getString(), "Player1");

    // Test Gravity component
    const auto& gravity = components[2];
    EXPECT_EQ(gravity["type"].getString(), "Gravity");
    const auto& force = gravity["data"]["force"];
    EXPECT_TRUE(approxEqual(force["x"].getNumber(), 0.0));
    EXPECT_TRUE(approxEqual(force["y"].getNumber(), -9.81));
}

TEST(JsonTest, ParseLevelFile)
{
    // Read the level.json test file
    std::string json = readFile("tests/test_data/level.json");
    JsonValue   root(json);

    // Test level properties
    EXPECT_EQ(root["name"].getString(), "Level 1");
    EXPECT_EQ(root["description"].getString(), "The first level of the game");

    // Test entities array
    const auto& entities = root["entities"].getArray();
    ASSERT_EQ(entities.size(), 3);

    // Test player entity
    const auto& player = entities[0];
    EXPECT_EQ(player["id"].getNumber(), 1);
    EXPECT_EQ(player["tag"].getString(), "player");

    // Test platform entity
    const auto& platform = entities[1];
    EXPECT_EQ(platform["id"].getNumber(), 2);
    EXPECT_EQ(platform["tag"].getString(), "platform");
    const auto& platformScale = platform["components"][0]["data"]["scale"];
    EXPECT_TRUE(approxEqual(platformScale["x"].getNumber(), 5.0));
    EXPECT_TRUE(approxEqual(platformScale["y"].getNumber(), 1.0));

    // Test enemy entity
    const auto& enemy = entities[2];
    EXPECT_EQ(enemy["id"].getNumber(), 3);
    EXPECT_EQ(enemy["tag"].getString(), "enemy");
    EXPECT_EQ(enemy["components"][1]["data"]["name"].getString(), "Enemy1");

    // Test level settings
    const auto& settings = root["settings"];
    EXPECT_EQ(settings["background"].getString(), "forest");
    EXPECT_EQ(settings["music"].getString(), "level1_theme");
    EXPECT_EQ(settings["time_limit"].getNumber(), 300);
    const auto& gravity = settings["gravity"];
    EXPECT_TRUE(approxEqual(gravity["x"].getNumber(), 0.0));
    EXPECT_TRUE(approxEqual(gravity["y"].getNumber(), -9.81));
}

TEST(JsonTest, ErrorHandling)
{
    // Test invalid JSON
    EXPECT_THROW(JsonValue("{invalid json}"), std::runtime_error);

    // Test accessing non-existent keys
    JsonValue empty("{}");
    EXPECT_EQ(empty["nonexistent"].isNull(), true);

    // Test accessing wrong types
    JsonValue num("42");
    EXPECT_EQ(num.getString("default"), "default");
    EXPECT_EQ(num.getBool(true), true);

    // Test array bounds
    JsonValue arr("[1,2,3]");
    EXPECT_EQ(arr[5].isNull(), true);
}

TEST(JsonTest, BuilderSimpleTypes)
{
    JsonBuilder builder;

    // Test object with simple types
    builder.beginObject();
    builder.addKey("string");
    builder.addString("Hello, World!");
    builder.addKey("number");
    builder.addNumber(42.5f);
    builder.addKey("boolean");
    builder.addBool(true);
    builder.endObject();

    // Parse the built JSON to verify structure
    JsonValue value(builder.toString());
    EXPECT_EQ(value["string"].getString(), "Hello, World!");
    EXPECT_TRUE(approxEqual(value["number"].getNumber(), 42.5));
    EXPECT_EQ(value["boolean"].getBool(), true);
}

TEST(JsonTest, BuilderArrays)
{
    JsonBuilder builder;

    // Test array with mixed types
    builder.beginArray();
    builder.addString("first");
    builder.addNumber(1.0f);
    builder.addBool(false);
    builder.beginObject();
    builder.addKey("nested");
    builder.addString("value");
    builder.endObject();
    builder.endArray();

    // Parse and verify
    JsonValue   value(builder.toString());
    const auto& arr = value.getArray();
    ASSERT_EQ(arr.size(), 4);
    EXPECT_EQ(arr[0].getString(), "first");
    EXPECT_TRUE(approxEqual(arr[1].getNumber(), 1.0));
    EXPECT_EQ(arr[2].getBool(), false);
    EXPECT_EQ(arr[3]["nested"].getString(), "value");
}

TEST(JsonTest, BuilderComplexStructure)
{
    JsonBuilder builder;

    // Build a complex entity structure similar to entity.json
    builder.beginObject();

    // Basic properties
    builder.addKey("id");
    builder.addNumber(1.0f);
    builder.addKey("tag");
    builder.addString("player");

    // Components array
    builder.addKey("components");
    builder.beginArray();

    // Transform component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Transform");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(100.0f);
    builder.addKey("y");
    builder.addNumber(200.0f);
    builder.endObject();
    builder.addKey("velocity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(0.0f);
    builder.addKey("y");
    builder.addNumber(0.0f);
    builder.endObject();
    builder.endObject();
    builder.endObject();

    // Name component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Name");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("name");
    builder.addString("Player1");
    builder.endObject();
    builder.endObject();

    builder.endArray();
    builder.endObject();

    // Parse and verify the complex structure
    JsonValue value(builder.toString());
    EXPECT_EQ(value["id"].getNumber(), 1.0);
    EXPECT_EQ(value["tag"].getString(), "player");

    const auto& components = value["components"].getArray();
    ASSERT_EQ(components.size(), 2);

    // Verify Transform component
    const auto& transform = components[0];
    EXPECT_EQ(transform["type"].getString(), "Transform");
    const auto& pos = transform["data"]["position"];
    EXPECT_TRUE(approxEqual(pos["x"].getNumber(), 100.0));
    EXPECT_TRUE(approxEqual(pos["y"].getNumber(), 200.0));

    // Verify Name component
    const auto& name = components[1];
    EXPECT_EQ(name["type"].getString(), "Name");
    EXPECT_EQ(name["data"]["name"].getString(), "Player1");
}

TEST(JsonTest, BuilderEscapeStrings)
{
    JsonBuilder builder;

    builder.beginObject();
    builder.addKey("special\nkey");           // Key with newline
    builder.addString("Hello\t\"World\"\n");  // String with tab, quotes and newline
    builder.addKey("path");
    builder.addString("C:\\Program Files\\Game");  // Windows path with backslashes
    builder.endObject();

    // Parse and verify escaped strings
    JsonValue value(builder.toString());
    EXPECT_EQ(value["special\nkey"].getString(), "Hello\t\"World\"\n");
    EXPECT_EQ(value["path"].getString(), "C:\\Program Files\\Game");
}

TEST(JsonTest, BuilderWriteAndReadFile)
{
    JsonBuilder builder;

    // Build a complete level structure
    builder.beginObject();

    // Level metadata
    builder.addKey("name");
    builder.addString("Test Level");
    builder.addKey("description");
    builder.addString("A test level created by JsonBuilder");

    // Entities array
    builder.addKey("entities");
    builder.beginArray();

    // Player entity
    builder.beginObject();
    builder.addKey("id");
    builder.addNumber(1.0f);
    builder.addKey("tag");
    builder.addString("player");
    builder.addKey("components");
    builder.beginArray();

    // Transform component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Transform");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(100.0f);
    builder.addKey("y");
    builder.addNumber(200.0f);
    builder.endObject();
    builder.addKey("scale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(1.0f);
    builder.addKey("y");
    builder.addNumber(1.0f);
    builder.endObject();
    builder.addKey("rotation");
    builder.addNumber(0.0f);
    builder.endObject();
    builder.endObject();

    // Name component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Name");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("name");
    builder.addString("Player1");
    builder.endObject();
    builder.endObject();

    // Gravity component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Gravity");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("force");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(0.0f);
    builder.addKey("y");
    builder.addNumber(-9.81f);
    builder.endObject();
    builder.endObject();
    builder.endObject();

    builder.endArray();   // end components
    builder.endObject();  // end player entity

    // Platform entity
    builder.beginObject();
    builder.addKey("id");
    builder.addNumber(2.0f);
    builder.addKey("tag");
    builder.addString("platform");
    builder.addKey("components");
    builder.beginArray();

    // Transform component
    builder.beginObject();
    builder.addKey("type");
    builder.addString("Transform");
    builder.addKey("data");
    builder.beginObject();
    builder.addKey("position");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(300.0f);
    builder.addKey("y");
    builder.addNumber(400.0f);
    builder.endObject();
    builder.addKey("scale");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(5.0f);
    builder.addKey("y");
    builder.addNumber(1.0f);
    builder.endObject();
    builder.addKey("rotation");
    builder.addNumber(0.0f);
    builder.endObject();
    builder.endObject();

    builder.endArray();   // end components
    builder.endObject();  // end platform entity

    builder.endArray();  // end entities

    // Level settings
    builder.addKey("settings");
    builder.beginObject();
    builder.addKey("gravity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(0.0f);
    builder.addKey("y");
    builder.addNumber(-9.81f);
    builder.endObject();
    builder.addKey("background");
    builder.addString("mountains");
    builder.addKey("music");
    builder.addString("test_theme");
    builder.addKey("time_limit");
    builder.addNumber(300.0f);
    builder.endObject();

    builder.endObject();  // end level

    // Write to file
    std::string outputPath = "tests/test_data/builder_output.json";
    writeFile(outputPath, builder.toString());

    // Read back and verify
    std::string json = readFile(outputPath);
    JsonValue   root(json);

    // Verify level metadata
    EXPECT_EQ(root["name"].getString(), "Test Level");
    EXPECT_EQ(root["description"].getString(), "A test level created by JsonBuilder");

    // Verify entities
    const auto& entities = root["entities"].getArray();
    ASSERT_EQ(entities.size(), 2);

    // Verify player entity
    const auto& player = entities[0];
    EXPECT_EQ(player["id"].getNumber(), 1.0);
    EXPECT_EQ(player["tag"].getString(), "player");
    const auto& playerComponents = player["components"].getArray();
    ASSERT_EQ(playerComponents.size(), 3);

    // Verify platform entity
    const auto& platform = entities[1];
    EXPECT_EQ(platform["id"].getNumber(), 2.0);
    EXPECT_EQ(platform["tag"].getString(), "platform");
    const auto& platformScale = platform["components"][0]["data"]["scale"];
    EXPECT_TRUE(approxEqual(platformScale["x"].getNumber(), 5.0));
    EXPECT_TRUE(approxEqual(platformScale["y"].getNumber(), 1.0));

    // Verify settings
    const auto& settings = root["settings"];
    EXPECT_EQ(settings["background"].getString(), "mountains");
    EXPECT_EQ(settings["music"].getString(), "test_theme");
    EXPECT_EQ(settings["time_limit"].getNumber(), 300.0);
    const auto& gravity = settings["gravity"];
    EXPECT_TRUE(approxEqual(gravity["x"].getNumber(), 0.0));
    EXPECT_TRUE(approxEqual(gravity["y"].getNumber(), -9.81));
}