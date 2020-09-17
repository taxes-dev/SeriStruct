#include "SeriStruct.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <sstream>

using namespace Catch::literals;
using SeriStruct::Record;

struct TestRecord : public Record
{
public:
    TestRecord(uint32_t a, int32_t b, float c, bool d, bool e, float f, char g)
        : Record{buffer_size}
    {
        assign_buffer(offset_a, a);
        assign_buffer(offset_b, b);
        assign_buffer(offset_c, c);
        assign_buffer(offset_d, d);
        assign_buffer(offset_e, e);
        assign_buffer(offset_f, f);
        assign_buffer(offset_g, g);
    }
    TestRecord(std::istream &istr)
        : Record{istr, buffer_size}
    {
    }
    TestRecord(const unsigned char *buffer, const size_t buffer_size) : Record{buffer, buffer_size, TestRecord::buffer_size}
    {
    }
    TestRecord(const TestRecord &other) : Record{other} {}
    TestRecord(TestRecord &&other) : Record{std::move(other)} {}
    ~TestRecord()
    {
    }

    inline uint32_t &a() const { return buffer_at<uint32_t>(offset_a); }
    inline int32_t &b() const { return buffer_at<int32_t>(offset_b); }
    inline float &c() const { return buffer_at<float>(offset_c); }
    inline bool &d() const { return buffer_at<bool>(offset_d); }
    inline bool &e() const { return buffer_at<bool>(offset_e); }
    inline float &f() const { return buffer_at<float>(offset_f); }
    inline char &g() const { return buffer_at<char>(offset_g); }

private:
    static constexpr size_t offset_a = 0;
    static constexpr size_t offset_b = offset_a + sizeof(uint32_t); // offset of previous field + size of previous field
    static constexpr size_t offset_c = offset_b + sizeof(int32_t);
    static constexpr size_t offset_d = offset_c + sizeof(float);
    static constexpr size_t offset_e = offset_d + sizeof(bool);
    static constexpr size_t offset_f = offset_e + sizeof(bool) + 2; // keep float 4-byte aligned
    static constexpr size_t offset_g = offset_f + sizeof(float);
    static constexpr size_t buffer_size = offset_g + sizeof(char); // the end of the struct
};

// Used in tests below, should be expected TestRecord.buffer_size
#define EXPECTED_BUFFER_SIZE 21UL

TEST_CASE("Allocate record with primitives")
{
    TestRecord record{5, -1, 3.0f, true, true, -1.5f, 'z'};

    REQUIRE(record.a() == 5);
    REQUIRE(record.b() == -1);
    REQUIRE(record.c() == 3.0_a);
    REQUIRE(record.d());
    REQUIRE(record.e());
    REQUIRE(record.f() == -1.5_a);
    REQUIRE(record.g() == 'z');

    REQUIRE(record.size() == EXPECTED_BUFFER_SIZE);
}

TEST_CASE("Write record to output stream")
{
    TestRecord record{3, -140, 0.0f, false, true, 14999.535f, 'Z'};

    // test writing to an ostream
    std::stringstream s;
    s << record;
    s.sync();

    // expect the buffer to include an additional 64-bits of size info
    REQUIRE(s.tellp() == EXPECTED_BUFFER_SIZE + sizeof(uint64_t));

    unsigned char EXPECTED_BYTES[] = {
        EXPECTED_BUFFER_SIZE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0x03, 0x00, 0x00, 0x00,                                         // a
        0x74, 0xff, 0xff, 0xff,                                         // b
        0x00, 0x00, 0x00, 0x00,                                         // c
        0x00, 0x01, 0x00, 0x00,                                         // d, e, padding
        0x24, 0x5e, 0x6a, 0x46,                                         // f
        0x5a                                                            // g
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

    REQUIRE(s2.tellp() == EXPECTED_BUFFER_SIZE + sizeof(uint64_t));

    s.seekg(0);
    s2.seekg(0);
    auto iter1 = std::istream_iterator<unsigned char>{s};
    auto iter2 = std::istream_iterator<unsigned char>{s2};
    for (; iter1 != std::istream_iterator<unsigned char>{} && iter2 != std::istream_iterator<unsigned char>{}; iter1++, iter2++)
    {
        REQUIRE(*iter1 == *iter2);
    }
}

TEST_CASE("Read record from input stream")
{
    std::stringstream s;
    unsigned char RECORD_BYTES[] = {
        EXPECTED_BUFFER_SIZE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0xe7, 0x03, 0x00, 0x00,                                         // a
        0x86, 0x05, 0x00, 0x00,                                         // b
        0x66, 0x66, 0x04, 0xc2,                                         // c
        0x01, 0x00, 0x00, 0x00,                                         // d, e, padding
        0x07, 0x1b, 0xb7, 0x49,                                         // f
        0x3f,                                                           // g
    };
    for (size_t i = 0; i < sizeof(RECORD_BYTES) / sizeof(unsigned char); i++)
    {
        s << RECORD_BYTES[i];
    }
    s.sync();
    s.seekg(0);

    TestRecord record{s};

    REQUIRE(record.a() == 999);
    REQUIRE(record.b() == 1414);
    REQUIRE(record.c() == -33.1_a);
    REQUIRE(record.d());
    REQUIRE_FALSE(record.e());
    REQUIRE(record.f() == 1500000.93_a);
    REQUIRE(record.g() == '?');

    REQUIRE(record.size() == EXPECTED_BUFFER_SIZE);
}

TEST_CASE("Read record with incorrect size throws exception")
{
    std::stringstream s;
    unsigned char RECORD_BYTES[] = {
        0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0xe7, 0x03, 0x00, 0x00,                         // a
        0x86, 0x05, 0x00,                               // b... ?
    };
    for (size_t i = 0; i < sizeof(RECORD_BYTES) / sizeof(unsigned char); i++)
    {
        s << RECORD_BYTES[i];
    }
    s.sync();
    s.seekg(0);

    REQUIRE_THROWS_AS(TestRecord(s), SeriStruct::invalid_size);
}

TEST_CASE("Read record with insufficent data throws exception")
{
    std::stringstream s;
    unsigned char RECORD_BYTES[] = {
        EXPECTED_BUFFER_SIZE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // size() as unit64_t
        0xe7, 0x03, 0x00, 0x00,                                         // a
        0x86, 0x05, 0x00,                                               // b... ?
    };
    for (size_t i = 0; i < sizeof(RECORD_BYTES) / sizeof(unsigned char); i++)
    {
        s << RECORD_BYTES[i];
    }
    s.sync();
    s.seekg(0);

    REQUIRE_THROWS_AS(TestRecord(s), SeriStruct::not_enough_data);
}

TEST_CASE("Copy to a buffer")
{
    TestRecord record{1997, 1883, -999.99f, true, false, 1.0f, '-'};

    unsigned char buffer[EXPECTED_BUFFER_SIZE];
    record.copy_to(buffer);

    unsigned char RECORD_BYTES[] = {
        0xcd, 0x07, 0x00, 0x00, // a
        0x5b, 0x07, 0x00, 0x00, // b
        0x5c, 0xff, 0x79, 0xc4, // c
        0x01, 0x00, 0x00, 0x00, // d, e, padding
        0x00, 0x00, 0x80, 0x3f, // f
        0x2d,                   // g
    };

    for (size_t i = 0; i < sizeof(RECORD_BYTES); i++)
    {
        REQUIRE(RECORD_BYTES[i] == buffer[i]);
    }
}

TEST_CASE("Copy from a buffer")
{
    unsigned char RECORD_BYTES[] = {
        0xcd, 0x07, 0x00, 0x00, // a
        0x5b, 0x07, 0x00, 0x00, // b
        0x5c, 0xff, 0x79, 0xc4, // c
        0x01, 0x00, 0x00, 0x00, // d, e, padding
        0x00, 0x00, 0x80, 0x3f, // f
        0x2d,                   // g
    };

    TestRecord record{RECORD_BYTES, EXPECTED_BUFFER_SIZE};

    REQUIRE(record.a() == 1997);
    REQUIRE(record.b() == 1883);
    REQUIRE(record.c() == -999.99_a);
    REQUIRE(record.d());
    REQUIRE_FALSE(record.e());
    REQUIRE(record.f() == 1.0_a);
    REQUIRE(record.g() == '-');
}

TEST_CASE("Copy from a buffer with incorrect size throws an exception")
{
    unsigned char RECORD_BYTES[] = {
        0x00,
        0x00,
        0x00,
        0x00,
    };

    REQUIRE_THROWS_AS(TestRecord(RECORD_BYTES, 4), SeriStruct::invalid_size);
}

TEST_CASE("Copy and move constructors")
{
    // copy constructor
    TestRecord record{34391, -5, 10.5f, true, true, -1111.0f, '('};
    TestRecord record2{record};

    REQUIRE(record.a() == record2.a());
    REQUIRE(record.b() == record2.b());
    REQUIRE(record.c() == record2.c());
    REQUIRE(record.d() == record2.d());
    REQUIRE(record.e() == record2.e());
    REQUIRE(record.f() == record2.f());
    REQUIRE(record.g() == record2.g());

    // move constructor
    TestRecord record3{std::move(record)};

    REQUIRE(&record != &record3);
    REQUIRE(record3.a() == record2.a());
    REQUIRE(record3.b() == record2.b());
    REQUIRE(record3.c() == record2.c());
    REQUIRE(record3.d() == record2.d());
    REQUIRE(record3.e() == record2.e());
    REQUIRE(record3.f() == record2.f());
    REQUIRE(record3.g() == record2.g());
}