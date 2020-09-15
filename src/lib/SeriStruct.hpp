#pragma once
#include <cstddef>
#include <type_traits>

namespace SeriStruct
{
    /**
     * @brief An immutable set of data that can be serialized/deserialized into raw bytes. Classes
     * that derive from Record should insert data in the constructor using Record::assign_buffer"()" and
     * implement getters that read from the internal buffer using Record::buffer_at"()".
     * 
     */
    class Record
    {
    public:
        /**
         * @brief Construct a new Record object
         * 
         * @param alloc_size The size of the internal buffer, which should be the sum of the size of all
         * the derived object's fields.
         */
        Record(const size_t alloc_size) : alloc_size{alloc_size} { alloc(); };

        /**
         * @brief Destroy the Record object
         */
        ~Record()
        {
            if (buffer)
            {
                delete[] buffer;
            }
        };

        /**
         * @brief Returns the size of the allocated internal buffer (size of the interior struct).
         * 
         * @return size_t 
         */
        inline size_t size() const { return alloc_size; };

    protected:
        /**
         * @brief Assigns a value to a particular offset in the buffer. Note that \p value must be an
         * integral or floating point.
         * 
         * @tparam T is the type of \p value
         * @param offset is the offset into the buffer 
         * @param value is the value, which overwrites any bytes at \p offset
         */
        template <typename T, typename = typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline void assign_buffer(const size_t &offset, const T &value)
        {
            *(reinterpret_cast<T *>(buffer + offset)) = value;
        }

        /**
         * @brief Gets a value at a particular offset in the buffer. Note that the return value must
         * be an integral or floating point.
         * 
         * @tparam T is the type of the return value
         * @param offset is the offset into the buffer
         * @return T& the value found at \p offset
         */
        template <typename T, typename = typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline T &buffer_at(const size_t &offset) const
        {
            return *(reinterpret_cast<T *>(buffer + offset));
        }

    private:
        const size_t alloc_size;
        unsigned char *buffer;
        void alloc()
        {
            buffer = new unsigned char[alloc_size];
        }
    };

} // namespace SeriStruct