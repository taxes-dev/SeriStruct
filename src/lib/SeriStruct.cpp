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
        std::memcpy(this->buffer, buffer, size());
    }

    void Record::from_stream(std::istream &istr, const size_t read_size)
    {
        istr.read(reinterpret_cast<char *>(buffer), read_size);
        if (istr.eof() && istr.fail())
        {
            throw not_enough_data{};
        }
    }

} // namespace SeriStruct