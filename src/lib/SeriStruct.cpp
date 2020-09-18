#include <cstring>
#include "SeriStruct.hpp"

namespace SeriStruct
{
    std::ostream &operator<<(std::ostream &ostr, const Record &record)
    {
        record.write(ostr);
        return ostr;
    }

    void Record::write(std::ostream &ostr) const
    {
        for (size_t i = 0; i < this->size() / sizeof(unsigned char); i++)
        {
            ostr << this->buffer[i];
        }
    }

    void Record::copy_to(unsigned char *buffer) const
    {
        std::memcpy(buffer, this->buffer, size());
    }

    void Record::from_array(const unsigned char *buffer, const size_t buffer_size)
    {
        if (buffer_size != size())
        {
            throw invalid_size{};
        }
        std::memcpy(this->buffer, buffer, size());
    }

    void Record::from_stream(std::istream &istr, const size_t read_size)
    {
        // first verify read_size matches what we expect
        if (read_size < size())
        {
            throw invalid_size{};
        }
        // then read the buffer in
        istr.read(reinterpret_cast<char *>(buffer), read_size);
        if (istr.eof() && istr.fail())
        {
            throw not_enough_data{};
        }
    }

} // namespace SeriStruct