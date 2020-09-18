#pragma once
#include <cstddef>
#include <exception>
#include <iostream>
#include <type_traits>
#include <utility>

namespace SeriStruct
{
    class Record;
    /**
     * @brief Writes a SeriStruct::Record to a std::ostream.
     * 
     * @return std::ostream&
     */
    std::ostream &operator<<(std::ostream &, const Record &);

    /**
     * @brief Exception thrown when reading a stream of bytes to a SeriStruct::Record
     * and the size in the stream doesn't match the expected size of the struct.
     */
    class invalid_size : public std::exception
    {
        const char *what() const throw()
        {
            return "Struct size mismatch";
        }
    };

    /**
     * @brief Exception thrown when reading a stream of bytes to a SeriStruct::Record
     * and EOF is reached before enough bytes could be read.
     */
    class not_enough_data : public std::exception
    {
        const char *what() const throw()
        {
            return "Not enough data in stream to fill struct";
        }
    };

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
         * @param alloc_size is the size of the internal buffer, which should be the sum of the size of all
         * the derived object's fields
         */
        Record(const size_t alloc_size) : alloc_size{alloc_size} { alloc(); };

        /**
         * @brief Construct a new Record object from a stream of bytes
         * 
         * @param istr is an open stream for reading the bytes
         * @param read_size is the numer of bytes to read from \p istr
         * @param alloc_size is the size of the internal buffer, which should be the sum of the size of
         * all the derived object's fields
         * 
         * @exception SeriStruct::invalid_size if the size header doesn't match the expected \p alloc_size
         * @exception SeriStruct::not_enough_data if EOF is reached on \p istr before all data could be read
         */
        Record(std::istream &istr, const size_t read_size, const size_t alloc_size) : Record{alloc_size}
        {
            from_stream(istr, read_size);
        }

        /**
         * @brief Construct a new Record object by copying from \p buffer.
         * 
         * @param buffer is a buffer of bytes that matches the underling struct (such as from copy_to"()")
         * @param buffer_size is the size of \p buffer
         * @param alloc_size is the size of the internal buffer, which should be the sum of the size of all
         * the derived object's fields
         */
        Record(const unsigned char *buffer, const size_t buffer_size, const size_t alloc_size) : Record{alloc_size}
        {
            from_array(buffer, buffer_size);
        }

        /**
         * @brief Construct a new Record object by copying \p other. Note that this operation will not likely be meaningful
         * if \p other and this instance are not the same derived type.
         * 
         * @param other 
         */
        Record(const Record &other) : Record{other.alloc_size}
        {
            if (&other == this)
            {
                return;
            }
            from_array(other.buffer, alloc_size);
        }

        /**
         * @brief Construct a new Record object by moving \p other. Note that this operation will not likely be meaningful
         * if \p other and this instance are not the same derived type. Attempts to access the fields of \p other after
         * the move operation are undefined behavior.
         * 
         * @param other 
         */
        Record(Record &&other) : Record{other.alloc_size}
        {
            if (&other == this)
            {
                return;
            }
            std::swap(buffer, other.buffer);
        }

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

        /**
         * @brief Writes this Record to \p ostr.
         * 
         * @param ostr is a std::ostream ready for writing
         */
        void write(std::ostream &ostr) const;

        friend std::ostream &operator<<(std::ostream &, const Record &);

        /**
         * @brief Copies the internal buffer representation of this record to \p buffer.
         * 
         * @param buffer is the destination buffer. Make sure at least size"()" bytes are available.
         */
        void copy_to(unsigned char *buffer) const;

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
            buffer = new unsigned char[alloc_size]();
        }
        void from_array(const unsigned char *buffer, const size_t buffer_size);
        void from_stream(std::istream &istr, const size_t read_size);
    };

} // namespace SeriStruct