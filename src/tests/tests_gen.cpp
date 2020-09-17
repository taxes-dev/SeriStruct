/**
 * @file tests_gen.cpp
 * @brief Tests executed against dynamically generated Records. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "GenRecordOne.gen.hpp"
#include <iterator>
#include <sstream>

using namespace Catch::literals;
using SeriStruct::Record;

// Used in tests below, should be expected GenRecordOne.buffer_size
#define G1_EXPECTED_BUFFER_SIZE 28UL

TEST_CASE("Generated record with primitives", "[generated]")
{
    GenRecordOne record{5, -1, 'a', true, 99999.99999, -1.5f};

    REQUIRE(record.uint_field() == 5);
    REQUIRE(record.int_field() == -1);
    REQUIRE(record.char_field() == 'a');
    REQUIRE(record.bool_field());
    REQUIRE(record.dbl_field() == 99999.99999_a);
    REQUIRE(record.float_field() == -1.5_a);

    REQUIRE(record.size() == G1_EXPECTED_BUFFER_SIZE);
}

TEST_CASE("Generated record write to output stream", "[generated]")
{
    GenRecordOne record{5, -1, 'a', true, 99999.99999, -1.5f};

    // test writing to an ostream
    std::stringstream s;
    s << record;
    s.sync();

    // expect the buffer to include an additional 64-bits of size info
    REQUIRE(s.tellp() == G1_EXPECTED_BUFFER_SIZE + sizeof(uint64_t));

    unsigned char EXPECTED_BYTES[] = {
        G1_EXPECTED_BUFFER_SIZE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0x05, 0x00, 0x00, 0x00,                                            // uint_field
        0xff, 0xff, 0xff, 0xff,                                            // int_field
        0x61, 0x01,                                                        // char_field, bool_field
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                // padding
        0xa5, 0x83, 0xf5, 0xff, 0xff, 0x69, 0xf8, 0x40,                    // dbl_field
        0x00, 0x00, 0xc0, 0xbf,                                            // float_field
    };
    unsigned char *p = EXPECTED_BYTES;
    s.seekg(0);
    auto iter = std::istream_iterator<unsigned char>{s};
    for (; iter != std::istream_iterator<unsigned char>{}; iter++, p++)
    {
        REQUIRE(*iter == *p);
    }

    // using write() or << should have identical behavior
    std::stringstream s2;
    record.write(s2);
    s2.sync();

    REQUIRE(s2.tellp() == G1_EXPECTED_BUFFER_SIZE + sizeof(uint64_t));

    s.seekg(0);
    s2.seekg(0);
    auto iter1 = std::istream_iterator<unsigned char>{s};
    auto iter2 = std::istream_iterator<unsigned char>{s2};
    for (; iter1 != std::istream_iterator<unsigned char>{} && iter2 != std::istream_iterator<unsigned char>{}; iter1++, iter2++)
    {
        REQUIRE(*iter1 == *iter2);
    }
}

TEST_CASE("Generated record read from input stream", "[generated]")
{
    std::stringstream s;
    unsigned char RECORD_BYTES[] = {
        G1_EXPECTED_BUFFER_SIZE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0x05, 0x00, 0x00, 0x00,                                            // uint_field
        0xff, 0xff, 0xff, 0xff,                                            // int_field
        0x61, 0x01,                                                        // char_field, bool_field
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                                // padding
        0xa5, 0x83, 0xf5, 0xff, 0xff, 0x69, 0xf8, 0x40,                    // dbl_field
        0x00, 0x00, 0xc0, 0xbf,                                            // float_field
    };
    for (size_t i = 0; i < sizeof(RECORD_BYTES) / sizeof(unsigned char); i++)
    {
        s << RECORD_BYTES[i];
    }
    s.sync();
    s.seekg(0);

    GenRecordOne record{s};

    REQUIRE(record.uint_field() == 5);
    REQUIRE(record.int_field() == -1);
    REQUIRE(record.char_field() == 'a');
    REQUIRE(record.bool_field());
    REQUIRE(record.dbl_field() == 99999.99999_a);
    REQUIRE(record.float_field() == -1.5_a);

    REQUIRE(record.size() == G1_EXPECTED_BUFFER_SIZE);
}
