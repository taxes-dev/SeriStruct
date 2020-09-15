#include "SeriStruct.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace Catch::literals;
using SeriStruct::Record;

struct TestRecord : public Record
{
public:
    TestRecord(uint32_t a, int32_t b, float_t c, bool d, bool e, float_t f, char g)
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
    ~TestRecord()
    {
    }

    inline uint32_t &a() const { return buffer_at<uint32_t>(offset_a); }
    inline int32_t &b() const { return buffer_at<int32_t>(offset_b); }
    inline float_t &c() const { return buffer_at<float_t>(offset_c); }
    inline bool &d() const { return buffer_at<bool>(offset_d); }
    inline bool &e() const { return buffer_at<bool>(offset_e); }
    inline float_t &f() const { return buffer_at<float_t>(offset_f); }
    inline char &g() const { return buffer_at<char>(offset_g); }

private:
    static constexpr size_t offset_a = 0;
    static constexpr size_t offset_b = offset_a + sizeof(uint32_t); // offset of previous field + size of previous field
    static constexpr size_t offset_c = offset_b + sizeof(int32_t);
    static constexpr size_t offset_d = offset_c + sizeof(float_t);
    static constexpr size_t offset_e = offset_d + sizeof(bool);
    static constexpr size_t offset_f = offset_e + sizeof(bool) + 2; // keep following float 4-byte aligned
    static constexpr size_t offset_g = offset_f + sizeof(float_t);
    static constexpr size_t buffer_size = offset_g + sizeof(char); // the end of the struct
};

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

    REQUIRE(record.size() == 21UL);
}