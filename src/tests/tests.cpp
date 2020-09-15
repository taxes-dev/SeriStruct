#include "SeriStruct.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace Catch::literals;

struct TestRecord
{
public:
    TestRecord(uint32_t a, int32_t b, float_t c, bool d){
        alloc();
        *(static_cast<uint32_t *>(buffer + offset_a)) = a;
        *(static_cast<int32_t *>(buffer + offset_b)) = b;
        *(static_cast<float_t *>(buffer + offset_c)) = c;
        *(static_cast<bool *>(buffer + offset_d)) = d;
    }
    ~TestRecord()
    {
        if (buffer) {
            operator delete(buffer);
        }
    }

    uint32_t a() const { return *(static_cast<uint32_t *>(buffer + offset_a)); }
    int32_t b() const { return *(static_cast<int32_t *>(buffer + offset_b)); }
    float_t c() const { return *(static_cast<float_t *>(buffer + offset_c)); }
    bool d() const { return *(static_cast<bool *>(buffer + offset_d)); }
private:
    static const size_t offset_a = 0;
    static const size_t offset_b = offset_a + sizeof(uint32_t); // offset of previous field + size of previous field
    static const size_t offset_c = offset_b + sizeof(int32_t);
    static const size_t offset_d = offset_c + sizeof(float_t);
    void * buffer = nullptr;
    void alloc()
    {
        // calculate width of the struct based on its contents
        size_t size = sizeof(uint32_t) + sizeof(int32_t) + sizeof(float_t) + sizeof(bool);
        buffer = operator new(size);
    }
};

TEST_CASE("Allocate records with primitives") {
    TestRecord record{5, -1, 3.0f, true};

    REQUIRE( record.a() == 5 );
    REQUIRE( record.b() == -1 );
    REQUIRE( record.c() == 3.0_a );
    REQUIRE( record.d() );
}