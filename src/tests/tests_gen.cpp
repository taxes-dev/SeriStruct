/**
 * @file tests_gen.cpp
 * @brief Tests executed against dynamically generated Records. ssgen.py should be run
 * on GenRecords.txt before running these tests.
 * 
 */
#include "SeriStruct.hpp"
#include "catch.hpp"
#include "GenRecordOne.gen.hpp"
#include "GenRecordTwo.gen.hpp"
#include "GenRecordThree.gen.hpp"
#include <iterator>
#include <sstream>

using namespace Catch::literals;
using Catch::WithinRel;
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

TEST_CASE("Generated record write to output stream", "[generated][stream]")
{
    GenRecordOne record{5, -1, 'a', true, 99999.99999, -1.5f};

    // test writing to an ostream
    std::stringstream s;
    s << record;
    s.sync();

    REQUIRE(s.tellp() == G1_EXPECTED_BUFFER_SIZE);

    unsigned char EXPECTED_BYTES[] = {
        0x05, 0x00, 0x00, 0x00,                         // uint_field
        0xff, 0xff, 0xff, 0xff,                         // int_field
        0x61, 0x01,                                     // char_field, bool_field
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // padding
        0xa5, 0x83, 0xf5, 0xff, 0xff, 0x69, 0xf8, 0x40, // dbl_field
        0x00, 0x00, 0xc0, 0xbf,                         // float_field
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

    REQUIRE(s2.tellp() == G1_EXPECTED_BUFFER_SIZE);

    s.seekg(0);
    s2.seekg(0);
    auto iter1 = std::istream_iterator<unsigned char>{s};
    auto iter2 = std::istream_iterator<unsigned char>{s2};
    for (; iter1 != std::istream_iterator<unsigned char>{} && iter2 != std::istream_iterator<unsigned char>{}; iter1++, iter2++)
    {
        REQUIRE(*iter1 == *iter2);
    }
}

TEST_CASE("Generated record read from input stream", "[generated][stream]")
{
    std::stringstream s;
    unsigned char RECORD_BYTES[] = {
        0x05, 0x00, 0x00, 0x00,                         // uint_field
        0xff, 0xff, 0xff, 0xff,                         // int_field
        0x61, 0x01,                                     // char_field, bool_field
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // padding
        0xa5, 0x83, 0xf5, 0xff, 0xff, 0x69, 0xf8, 0x40, // dbl_field
        0x00, 0x00, 0xc0, 0xbf,                         // float_field
    };
    for (size_t i = 0; i < sizeof(RECORD_BYTES) / sizeof(RECORD_BYTES[0]); i++)
    {
        s << RECORD_BYTES[i];
    }
    s.sync();
    size_t stream_len = s.tellp();
    s.seekg(0);

    GenRecordOne record{s, stream_len};

    REQUIRE(record.uint_field() == 5);
    REQUIRE(record.int_field() == -1);
    REQUIRE(record.char_field() == 'a');
    REQUIRE(record.bool_field());
    REQUIRE(record.dbl_field() == 99999.99999_a);
    REQUIRE(record.float_field() == -1.5_a);

    REQUIRE(record.size() == G1_EXPECTED_BUFFER_SIZE);
}

TEST_CASE("Generated copy and move constructors", "[generated]")
{
    // copy constructor
    GenRecordOne record{5, -1, 'a', true, 99999.99999, -1.5f};
    GenRecordOne record2{record};

    REQUIRE(record.uint_field() == record2.uint_field());
    REQUIRE(record.int_field() == record2.int_field());
    REQUIRE(record.char_field() == record2.char_field());
    REQUIRE(record.bool_field() == record2.bool_field());
    REQUIRE_THAT(record.dbl_field(), WithinRel(record2.dbl_field()));
    REQUIRE_THAT(record.float_field(), WithinRel(record2.float_field()));

    // move constructor
    GenRecordOne record3{std::move(record)};

    REQUIRE(&record != &record3);
    REQUIRE(record3.uint_field() == record2.uint_field());
    REQUIRE(record3.int_field() == record2.int_field());
    REQUIRE(record3.char_field() == record2.char_field());
    REQUIRE(record3.bool_field() == record2.bool_field());
    REQUIRE_THAT(record3.dbl_field(), WithinRel(record2.dbl_field()));
    REQUIRE_THAT(record3.float_field(), WithinRel(record2.float_field()));
}

TEST_CASE("Generated copy assignment operator", "[generated]")
{
    GenRecordOne record{5, -1, 'a', true, 99999.99999, -1.5f};
    GenRecordOne record2{1997, -1883, '-', false, -999.99f, 1.0f};

    record2 = record;

    REQUIRE(record2.uint_field() == 5);
    REQUIRE(record2.int_field() == -1);
    REQUIRE(record2.char_field() == 'a');
    REQUIRE(record2.bool_field());
    REQUIRE(record2.dbl_field() == 99999.99999_a);
    REQUIRE(record2.float_field() == -1.5_a);
}

TEST_CASE("Generated move assignment operator", "[generated]")
{
    GenRecordOne record{ 5, -1, 'a', true, 99999.99999, -1.5f };
    GenRecordOne record2{ 1997, -1883, '-', false, -999.99f, 1.0f };

    record2 = std::move(record);

    REQUIRE(record2.uint_field() == 5);
    REQUIRE(record2.int_field() == -1);
    REQUIRE(record2.char_field() == 'a');
    REQUIRE(record2.bool_field());
    REQUIRE(record2.dbl_field() == 99999.99999_a);
    REQUIRE(record2.float_field() == -1.5_a);
}

TEST_CASE("Stream forward compatible records", "[generated][stream]")
{
    std::stringstream s, s2;

    // GenRecordThree represents a theoretical record that is forward-compatible
    // with GenRecordTwo (i.e. only adds new fields)
    GenRecordThree record{1, -1, 'b', true, 0xdead, 0xbeef};

    record.write(s);
    s.sync();
    auto record_len = s.tellp();
    REQUIRE(record_len > 0);
    s.seekg(0);

    GenRecordTwo record2{s, static_cast<size_t>(record_len)};
    REQUIRE(record.size() == record2.size());
    REQUIRE(record.uint_field() == record2.uint_field());
    REQUIRE(record.int_field() == record2.int_field());
    REQUIRE(record.char_field() == record2.char_field());
    REQUIRE(record.bool_field() == record2.bool_field());

    record2.write(s2);
    s2.sync();
    auto record_len_2 = s2.tellp();
    REQUIRE(record_len == record_len_2);
    s2.seekg(0);

    // The instantiation of GenRecordTwo from a GenRecordThree should preserve the
    // data in the extra fields, even though they're inaccessible
    GenRecordThree record3{s2, static_cast<size_t>(record_len_2)};
    REQUIRE(record.size() == record3.size());
    REQUIRE(record.uint_field() == record3.uint_field());
    REQUIRE(record.int_field() == record3.int_field());
    REQUIRE(record.char_field() == record3.char_field());
    REQUIRE(record.bool_field() == record3.bool_field());
    REQUIRE(record.uint_field_2() == record3.uint_field_2());
    REQUIRE(record.uint_field_3() == record3.uint_field_3());
}

TEST_CASE("Buffer copy forward compatible records", "[generated][buffer]")
{
    // GenRecordThree represents a theoretical record that is forward-compatible
    // with GenRecordTwo (i.e. only adds new fields)
    GenRecordThree record{1, -1, 'b', true, 0xdead, 0xbeef};

    auto buffer = new unsigned char[record.size()];
    record.copy_to(buffer);

    GenRecordTwo record2{buffer, record.size()};
    REQUIRE(record.size() == record2.size());
    REQUIRE(record.uint_field() == record2.uint_field());
    REQUIRE(record.int_field() == record2.int_field());
    REQUIRE(record.char_field() == record2.char_field());
    REQUIRE(record.bool_field() == record2.bool_field());

    auto buffer2 = new unsigned char[record2.size()];
    record2.copy_to(buffer2);

    // The instantiation of GenRecordTwo from a GenRecordThree should preserve the
    // data in the extra fields, even though they're inaccessible
    GenRecordThree record3{buffer2, record2.size()};
    REQUIRE(record.size() == record3.size());
    REQUIRE(record.uint_field() == record3.uint_field());
    REQUIRE(record.int_field() == record3.int_field());
    REQUIRE(record.char_field() == record3.char_field());
    REQUIRE(record.bool_field() == record3.bool_field());
    REQUIRE(record.uint_field_2() == record3.uint_field_2());
    REQUIRE(record.uint_field_3() == record3.uint_field_3());

    delete[] buffer, buffer2;
}