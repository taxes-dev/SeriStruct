/**
 * @file tests_arr_opt.cpp
 * @brief Tests for Records using array and optional. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "ArrayRecord.gen.hpp"
#include "OptionalRecord.gen.hpp"
#include "OptionalArrayRecord.gen.hpp"
#include <iterator>
#include <sstream>

using namespace Catch::literals;
using Catch::WithinRel;
using SeriStruct::Record;

TEST_CASE("Record with arrays", "[array]")
{
    std::array first_array{1, 2, 3};
    std::array second_array{'a', 'b', 'c', 'd', 'e'};
    std::array third_array{-1.0f, 999.99f};
    ArrayRecord record{first_array, 4, second_array, third_array};

    for (size_t i = 0; i < first_array.size(); i++)
    {
        REQUIRE(record.first_array()[i] == first_array[i]);
    }
    REQUIRE(record.int_field() == 4);
    for (size_t i = 0; i < second_array.size(); i++)
    {
        REQUIRE(record.second_array()[i] == second_array[i]);
    }
    for (size_t i = 0; i < third_array.size(); i++)
    {
        REQUIRE(record.third_array()[i] == third_array[i]);
    }
}

TEST_CASE("Array record read/write to output stream", "[array]")
{
    std::stringstream s;

    std::array first_array{128, -256, 512};
    std::array second_array{'1', '2', '3', '4', '5'};
    std::array third_array{1024.5f, -789.0f};
    ArrayRecord record{first_array, -23, second_array, third_array};

    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    ArrayRecord record2{s, static_cast<size_t>(record_len)};
    REQUIRE(record.first_array() == record2.first_array());
    REQUIRE(record.int_field() == record2.int_field());
    REQUIRE(record.second_array() == record2.second_array());
    REQUIRE(record.third_array() == record2.third_array());
}

TEST_CASE("Array record copy to/from a buffer", "[array]")
{
    std::array first_array{99, 100, 101};
    std::array second_array{' ', '-', '@', '$', '#'};
    std::array third_array{-1.0f, 1.0f};
    ArrayRecord record{first_array, 9999, second_array, third_array};

    unsigned char buffer[record.size()];
    record.copy_to(buffer);

    ArrayRecord record2{buffer, record.size()};
    REQUIRE(record.first_array() == record2.first_array());
    REQUIRE(record.int_field() == record2.int_field());
    REQUIRE(record.second_array() == record2.second_array());
    REQUIRE(record.third_array() == record2.third_array());
}


TEST_CASE("Record with optional", "[optional]")
{
    std::optional<char> not_present{};
    std::optional<uint> present{4};
    OptionalRecord record{not_present, present};
    REQUIRE_FALSE(record.first_opt());
    REQUIRE(record.second_opt());
    REQUIRE(record.second_opt().value() == 4);
}

TEST_CASE("Optional record read/write to output stream", "[optional]")
{
    std::stringstream s;
    std::optional<char> present{'p'};
    std::optional<uint> not_present{};
    OptionalRecord record{present, not_present};

    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    OptionalRecord record2{s, static_cast<size_t>(record_len)};
    REQUIRE(record2.first_opt());
    REQUIRE(record2.first_opt().value() == 'p');
    REQUIRE_FALSE(record2.second_opt());
}

TEST_CASE("Optional record copy to/from a buffer", "[optional]")
{
    std::optional<char> present1{'c'};
    std::optional<uint> present2{999};
    OptionalRecord record{present1, present2};

    unsigned char buffer[record.size()];
    record.copy_to(buffer);

    OptionalRecord record2{buffer, record.size()};
    REQUIRE(record2.first_opt());
    REQUIRE(record2.first_opt().value() == 'c');
    REQUIRE(record2.second_opt());
    REQUIRE(record2.second_opt().value() == 999);
}

TEST_CASE("Record with optional array", "[optional][array]")
{
    std::array arr = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    std::optional<std::array<int32_t, 10>> opt_arr{arr};
    OptionalArrayRecord record{true, opt_arr};

    REQUIRE(record.bool_field());
    REQUIRE(record.opt_array_field());
    REQUIRE(record.opt_array_field().value() == arr);
}

TEST_CASE("Optional array record read/write to output stream", "[optional][array]")
{
    std::stringstream s;

    std::array arr = {99, 98, 97, 96, 95, 94, 93, 92, 91, 90};
    std::optional<std::array<int32_t, 10>> opt_arr{arr};
    OptionalArrayRecord record{false, opt_arr};

    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    OptionalArrayRecord record2{s, static_cast<size_t>(record_len)};
    REQUIRE_FALSE(record2.bool_field());
    REQUIRE(record2.opt_array_field());
    REQUIRE(record2.opt_array_field().value() == arr);
}

TEST_CASE("Optional array record copy to/from a buffer", "[optional][array]")
{
    std::array arr = {1, -2, 3, -4, 5, -6, 7, -8, 9, -10};
    std::optional<std::array<int32_t, 10>> opt_arr{arr};
    OptionalArrayRecord record{true, opt_arr};

    unsigned char buffer[record.size()];
    record.copy_to(buffer);

    OptionalArrayRecord record2{buffer, record.size()};
    REQUIRE(record2.bool_field());
    REQUIRE(record2.opt_array_field());
    REQUIRE(record2.opt_array_field().value() == arr);
}