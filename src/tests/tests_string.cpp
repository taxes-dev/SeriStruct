/**
 * @file tests_string.cpp
 * @brief Tests for Records using C++ and C-style strings. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "CStringRecord.gen.hpp"
#include "StringRecord.gen.hpp"
#include <cstring>
#include <sstream>
#include <string>

using namespace Catch::literals;
using namespace std::string_literals;

TEST_CASE("Record with C strings", "[cstring]")
{
    CStringRecord record{'c', "this is a test string", nullptr, 1};
    REQUIRE(record.char_field() == 'c');
    REQUIRE(strcmp("this is a test string", record.cstr_field_1()) == 0);
    REQUIRE(record.cstr_field_2() == nullptr);
    REQUIRE(record.int_field() == 1);
}

TEST_CASE("C string record with field that exceeds max length is truncated", "[cstring]")
{
    CStringRecord record{'-', "The quick brown fox jumps over the lazy dog", "Hello world", 99};
    REQUIRE(strlen(record.cstr_field_1()) == 30);
    REQUIRE(strcmp("The quick brown fox jumps over", record.cstr_field_1()) == 0);
}

TEST_CASE("C string record read/write to output stream", "[cstring][stream]")
{
    std::stringstream s;

    CStringRecord record{'?', "hello world", "To be or not to be that is the question", 2};

    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    CStringRecord record2{s, static_cast<size_t>(record_len)};
    REQUIRE(record2.char_field() == '?');
    REQUIRE(strcmp("hello world", record2.cstr_field_1()) == 0);
    REQUIRE(strcmp("To be or not to be that is the question", record2.cstr_field_2()) == 0);
    REQUIRE(record2.int_field() == 2);
}

TEST_CASE("C string record copy to/from a buffer", "[cstring][buffer]")
{
    CStringRecord record{'*', nullptr, "What is man? A miserable little pile of secrets", -999};

    auto buffer = new unsigned char[record.size()];
    record.copy_to(buffer);

    CStringRecord record2{buffer, record.size()};
    REQUIRE(record2.char_field() == '*');
    REQUIRE(record2.cstr_field_1() == nullptr);
    REQUIRE(strcmp(record2.cstr_field_2(), "What is man? A miserable little pile of secrets") == 0);
    REQUIRE(record2.int_field() == -999);
}

TEST_CASE("Record with C++ strings", "[string]")
{
    StringRecord record{false, "this is a test string"s, "this is also a test string"s, 1.0f};
    REQUIRE_FALSE(record.bool_field());
    REQUIRE(record.str_field_1() == "this is a test string"s);
    REQUIRE(record.str_field_2() == "this is also a test string"s);
    REQUIRE(record.float_field() == 1.0_a);
}

TEST_CASE("String record with field that exceeds max length is truncated", "[string]")
{
    StringRecord record{false, "The quick brown fox jumps over the lazy dog"s, "hello world"s, 1.0f};
    REQUIRE(record.str_field_1().length() == 30);
    REQUIRE(record.str_field_1() == "The quick brown fox jumps over"s);
}

TEST_CASE("String record read/write to output stream", "[string][stream]")
{
    std::stringstream s;

    StringRecord record{true, "Hello world"s, "What's in a name? A rose by any name would smell as sweet."s, -99.99f};
    
    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    StringRecord record2{s, static_cast<size_t>(record_len)};
    REQUIRE(record2.bool_field());
    REQUIRE(record2.str_field_1() == "Hello world"s);
    REQUIRE(record2.str_field_2() == "What's in a name? A rose by any name would smell as sweet."s);
    REQUIRE(record2.float_field() == -99.99_a);
}

TEST_CASE("String record copy to/from a buffer", "[string][buffer]")
{
    StringRecord record{true, "All your base are belong to us", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 1024.1f};

    auto buffer = new unsigned char[record.size()];
    record.copy_to(buffer);

    StringRecord record2{buffer, record.size()};
    REQUIRE(record2.bool_field());
    REQUIRE(record2.str_field_1() == "All your base are belong to us"s);
    REQUIRE(record2.str_field_2() == "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"s);
    REQUIRE(record2.float_field() == 1024.1_a);
}