/**
 * @file tests_mut.cpp
 * @brief Tests for Records with mutable fields. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */

#include "SeriStruct.hpp"
#include "catch.hpp"
#include "MutableRecord.gen.hpp"
#include <cstring>

using namespace Catch::literals;
using namespace std::string_literals;

TEST_CASE("Record with mutable fields", "[mutable]")
{
    MutableRecord record{1, 1.0f, 'a', false, "Hello world", "Hello world 2"};
    record.int_field(-1);
    record.float_field(-999.99f);
    record.char_field('?');
    record.bool_field(true);
    record.cstr_field("The evil that men do lives after them; the good is oft interred with their bones.");
    record.str_field("Thank you Mario! But our princess is in another castle!"s);

    REQUIRE(record.int_field() == -1);
    REQUIRE(record.float_field() == -999.99_a);
    REQUIRE(record.char_field() == '?');
    REQUIRE(record.bool_field());
    REQUIRE(strcmp(record.cstr_field(), "The evil that men do lives after them; the good is oft interred with their bones.") == 0);
    REQUIRE(record.str_field() == "Thank you Mario! But our princess is in another castle!"s);
}

TEST_CASE("Mutable string field set to exceed max length is truncated", "[mutable][string]")
{
    MutableRecord record{1, 1.0f, 'a', false, "The quick brown fox jumps over the lazy dog", "Hello world"s};
    record.str_field("We the People of the United States, in Order to form a more perfect Union, establish Justice, insure domestic Tranquility, provide for the common defense, promote the general Welfare,"s);

    REQUIRE(record.str_field().length() == 60);
    REQUIRE(record.str_field() == "We the People of the United States, in Order to form a more "s);
}

TEST_CASE("Mutable C string field set to exceed max length is truncated", "[mutable][cstring]")
{
    MutableRecord record{1, 1.0f, 'a', false, "The quick brown fox jumps over the lazy dog", "Hello world"s};
    record.cstr_field("Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.");

    REQUIRE(strlen(record.cstr_field()) == 90);
    REQUIRE(strcmp("Four score and seven years ago our fathers brought forth on this continent, a new nation, ", record.cstr_field()) == 0);
}