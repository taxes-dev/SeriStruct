/**
 * @file tests_string.cpp
 * @brief Tests for Records using C++ and C-style strings. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "CStringRecord.gen.hpp"
#include <cstring>
#include <sstream>

using namespace Catch::literals;

TEST_CASE("Record with C strings", "[cstring]")
{
    CStringRecord record{'c', "this is a test string", nullptr, 1};
    REQUIRE(record.char_field() == 'c');
    REQUIRE(strcmp("this is a test string", record.cstr_field_1()) == 0);
    REQUIRE(record.cstr_field_2() == nullptr);
    REQUIRE(record.int_field() == 1);
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

    unsigned char buffer[record.size()];
    record.copy_to(buffer);

    CStringRecord record2{buffer, record.size()};
    REQUIRE(record2.char_field() == '*');
    REQUIRE(record2.cstr_field_1() == nullptr);
    REQUIRE(strcmp(record2.cstr_field_2(), "What is man? A miserable little pile of secrets") == 0);
    REQUIRE(record2.int_field() == -999);
}