#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
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
     * that derive from Record should insert data in the constructor using Record::assign_buffer() and
     * implement getters that read from the internal buffer using Record::buffer_at().
     * 
     */
    class Record
    {
    public:
        /**
         * @brief Construct a new Record object from a stream of bytes
         * 
         * @param istr is an open stream for reading the bytes
         * @param read_size is the numer of bytes to read from \p istr
         * @param expected_size is the minimum size of data this struct expects
         * 
         * @exception SeriStruct::invalid_size if \p read_size < \p expected_size
         * @exception SeriStruct::not_enough_data if EOF is reached on \p istr before all data could be read
         */
        Record(std::istream &istr, const size_t read_size, const size_t expected_size) : Record{}
        {
            if (read_size < expected_size)
            {
                throw invalid_size{};
            }
            alloc(read_size);
            from_stream(istr, read_size);
        }

        /**
         * @brief Construct a new Record object by copying from \p buffer.
         * 
         * @param buffer is a buffer of bytes that matches the underling struct (such as from copy_to())
         * @param buffer_size is the size of \p buffer
         * @param expected_size is the minimum size of data this struct expects
         * 
         * @exception SeriStruct::invalid_size if \p buffer_size < \p expected_size
         */
        Record(const unsigned char *buffer, const size_t buffer_size, const size_t expected_size) : Record{}
        {
            if (buffer_size < expected_size)
            {
                throw invalid_size{};
            }
            alloc(buffer_size);
            from_array(buffer, buffer_size);
        }

        /**
         * @brief Construct a new Record object by copying \p other. Note that this operation will not likely be meaningful
         * if \p other and this instance are not the same derived type.
         * 
         * @param other 
         */
        Record(const Record &other) : Record{}
        {
            if (&other != this)
            {
                alloc(other.alloc_size);
                from_array(other.buffer, alloc_size);
            }
        }

        /**
         * @brief Copy assignment operator
         * 
         * @param other 
         * @return Record& 
         */
        Record &operator=(const Record &other)
        {
            if (&other != this)
            {
                alloc(other.alloc_size);
                from_array(other.buffer, other.alloc_size);
            }
            return *this;
        }

        /**
         * @brief Construct a new Record object by moving \p other. Note that this operation will not likely be meaningful
         * if \p other and this instance are not the same derived type. Attempts to access the fields of \p other after
         * the move operation are undefined behavior.
         * 
         * @param other 
         */
        Record(Record &&other) : Record{}
        {
            if (&other != this)
            {
                std::swap(alloc_size, other.alloc_size);
                std::swap(buffer, other.buffer);
            }
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
         * @param buffer is the destination buffer. Make sure at least size() bytes are available.
         */
        void copy_to(unsigned char *buffer) const;

    protected:
        /**
         * @brief Construct a new Record object
         * 
         */
        Record() : alloc_size{0}, buffer{nullptr} {}

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
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + sizeof(T) <= alloc_size));
            *(reinterpret_cast<T *>(buffer + offset)) = value;
        }

        template <typename T, size_t N, typename = typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline void assign_buffer(const size_t &offset, const std::array<T, N> &value)
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + sizeof(T) * N <= alloc_size));
            *(reinterpret_cast<std::array<T, N> *>(buffer + offset)) = value;
        }

        template<typename T>
        inline void assign_buffer(const size_t &offset, const std::optional<T> &value)
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + sizeof(std::optional<T>) <= alloc_size));
            *(reinterpret_cast<std::optional<T> *>(buffer + offset)) = value;
        }

        /**
         * @brief Gets a value at a particular offset in the buffer. Note that the return value must
         * be an integral or floating point.
         * 
         * @tparam T is the type of the return value
         * @param offset is the offset into the buffer
         * @return T& the value found at \p offset
         */
        template <typename T>
        inline T &buffer_at(const size_t &offset) const
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to read past end of buffer", offset + sizeof(T) <= alloc_size));
            return *(reinterpret_cast<T *>(buffer + offset));
        }

        /**
         * @brief Allocates the underlying buffer.
         * 
         * @param alloc_size is the size to allocate in bytes
         */
        void alloc(const size_t &alloc_size)
        {
            this->alloc_size = alloc_size;
            if (buffer)
            {
                delete[] buffer;
            }
            buffer = new unsigned char[alloc_size]();
        }

    private:
        size_t alloc_size;
        unsigned char *buffer;
        void from_array(const unsigned char *buffer, const size_t buffer_size);
        void from_stream(std::istream &istr, const size_t read_size);
    };

} // namespace SeriStruct