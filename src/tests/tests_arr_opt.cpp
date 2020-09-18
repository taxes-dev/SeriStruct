/**
 * @file tests_arr_opt.cpp
 * @brief Tests for Records using array and optional. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "ArrayRecord.gen.hpp"
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
