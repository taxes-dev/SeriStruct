#include "SeriStruct.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace Catch::literals;

struct TestRecord
{
public:
    TestRecord(uint32_t a, int32_t b, float_t c, bool d, bool e, float_t f, char g)
    {
        alloc();
        *(reinterpret_cast<uint32_t *>(buffer + offset_a)) = a;
        *(reinterpret_cast<int32_t *>(buffer + offset_b)) = b;
        *(reinterpret_cast<float_t *>(buffer + offset_c)) = c;
        *(reinterpret_cast<bool *>(buffer + offset_d)) = d;
        *(reinterpret_cast<bool *>(buffer + offset_e)) = e;
        *(reinterpret_cast<float_t *>(buffer + offset_f)) = f;
        *(reinterpret_cast<char *>(buffer + offset_g)) = g;
    }
    ~TestRecord()
    {
        if (buffer)
        {
            operator delete(buffer);
        }
    }

    uint32_t &a() const { return *(reinterpret_cast<uint32_t *>(buffer + offset_a)); }
    int32_t &b() const { return *(reinterpret_cast<int32_t *>(buffer + offset_b)); }
    float_t &c() const { return *(reinterpret_cast<float_t *>(buffer + offset_c)); }
    bool &d() const { return *(reinterpret_cast<bool *>(buffer + offset_d)); }
    bool &e() const { return *(reinterpret_cast<bool *>(buffer + offset_e)); }
    float_t &f() const { return *(reinterpret_cast<float_t *>(buffer + offset_f)); }
    char &g() const { return *(reinterpret_cast<char *>(buffer + offset_g)); }

    size_t size() const { return buffer_size; }

private:
    static constexpr size_t offset_a = 0;
    static constexpr size_t offset_b = offset_a + sizeof(uint32_t); // offset of previous field + size of previous field
    static constexpr size_t offset_c = offset_b + sizeof(int32_t);
    static constexpr size_t offset_d = offset_c + sizeof(float_t);
    static constexpr size_t offset_e = offset_d + sizeof(bool);
    static constexpr size_t offset_f = offset_e + sizeof(bool) + 2; // keep following float 4-byte aligned
    static constexpr size_t offset_g = offset_f + sizeof(float_t);
    static constexpr size_t buffer_size = offset_g + sizeof(char); // the end of the struct
    unsigned char *buffer = nullptr;
    void alloc()
    {
        buffer = new unsigned char[buffer_size];
    }
};

TEST_CASE("Allocate records with primitives")
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