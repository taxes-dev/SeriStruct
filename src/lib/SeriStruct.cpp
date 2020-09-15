#include "SeriStruct.hpp"

namespace SeriStruct
{
    std::ostream &operator<<(std::ostream &ostr, const Record &record)
    {
        // first write the size of buffer, normalize to a 64-bit value
        uint64_t size = static_cast<uint64_t>(record.size());
        unsigned char *size_bytes = reinterpret_cast<unsigned char *>(&size);
        for (size_t i = 0; i < sizeof(uint64_t); i++)
        {
            ostr << size_bytes[i];
        }

        // then write the individual bytes
        for (size_t i = 0; i < record.size() / sizeof(unsigned char); i++)
        {
            ostr << record.buffer[i];
        }
        return ostr;
    }

    void Record::from_stream(std::istream &istr)
    {
        // first read the size and verify
        unsigned char size_bytes[sizeof(uint64_t)];
        istr.read(reinterpret_cast<char *>(&size_bytes), sizeof(uint64_t));
        uint64_t * size = reinterpret_cast<uint64_t *>(&size_bytes);
        if (*size != this->size())
        {
            throw invalid_size{};
        }
        istr.read(reinterpret_cast<char *>(buffer), *size);
        if (istr.eof() && istr.fail())
        {
            throw not_enough_data{};
        }
    }

} // namespace SeriStruct