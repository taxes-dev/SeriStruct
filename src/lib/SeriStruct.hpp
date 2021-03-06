#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <optional>
#include <string_view>
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
     * @brief A set of data that can be serialized/deserialized into raw bytes. Classes
     * that derive from Record should insert data in the constructor using Record::assign_buffer() and
     * implement getters that read from the internal buffer using Record::buffer_at().
     * Optionally setters can be implemented with the same calls to Record::assign_buffer().
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
        Record(Record &&other) noexcept : Record{}
        {
            if (&other != this)
            {
                std::swap(alloc_size, other.alloc_size);
                std::swap(buffer, other.buffer);
            }
        }

        /**
         * @brief Move assignment operator
         *
         * @param other
         * @return Record&
         */
        Record& operator=(Record&& other) noexcept {
            if (&other != this)
            {
                std::swap(alloc_size, other.alloc_size);
                std::swap(buffer, other.buffer);
            }
            return *this;
        }

        /**
         * @brief Destroy the Record object
         */
        virtual ~Record() noexcept
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
        Record() noexcept : alloc_size{0}, buffer{nullptr} {}

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

        /**
         * @brief Assigns an array of values to a particular offset in the buffer. Note that \p value must be
         * a std::array of integral or floating point.
         * 
         * @tparam T is the type of values in the array
         * @tparam N is the size of the array
         * @param offset is the offset into the buffer
         * @param value is the value, which overwrites any bytes at \p offset
         */
        template <typename T, size_t N, typename = typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline void assign_buffer(const size_t &offset, const std::array<T, N> &value)
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + sizeof(T) * N <= alloc_size));
            *(reinterpret_cast<std::array<T, N> *>(buffer + offset)) = value;
        }

        /**
         * @brief Assign an optional value to a particular offset in the buffer. \p value can be an integral,
         * floating point, or a std::array of such.
         * 
         * @tparam T is the type of value in the optional
         * @param offset is the offset into the buffer
         * @param value is the value, which overwrite any bytes at \p offset
         */
        template <typename T>
        inline void assign_buffer(const size_t &offset, const std::optional<T> &value)
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + sizeof(std::optional<T>) <= alloc_size));
            *(reinterpret_cast<std::optional<T> *>(buffer + offset)) = value;
        }

        /**
         * @brief Assign a C string to a particular offset in the buffer. \p value must be NUL-terminated.
         * 
         * @param offset is the offset into the buffer
         * @param value is the string, which overwrites any bytes at \p offset. May be nullptr.
         * @param maxlen is the maximum length allowed for the field. Excess bytes in \p value will be truncated.
         */
        inline void assign_buffer(const size_t &offset, const char *value, const size_t &maxlen)
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to write past end of buffer", offset + maxlen + alignof(char *) + 1 <= alloc_size));
            if (value == nullptr)
            {
                *(reinterpret_cast<char *>(buffer + offset)) = false;
            }
            else
            {
                *(reinterpret_cast<char *>(buffer + offset)) = true;
                strncpy(reinterpret_cast<char *>(buffer + offset + alignof(char *)), value, std::min(maxlen, strlen(value)));
            }
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
         * @brief Gets a C string at a particular offset in the buffer.
         * 
         * @param offset is the offset into the buffer
         * @return const char* the C string at \p offset. May be nullptr.
         */
        inline const char *buffer_at_cstr(const size_t &offset) const
        {
            assert(("Buffer was not allocated", buffer));
            assert(("Attempt to read past end of buffer", offset + alignof(char *) + sizeof(char *) <= alloc_size));
            bool is_present = buffer_at<bool>(offset);
            if (is_present)
            {
                return reinterpret_cast<const char *>(buffer + offset + alignof(char *));
            }
            else
            {
                return nullptr;
            }
        }

        /**
         * @brief Gets a string view at a particular offset in the buffer.
         * 
         * @param offset is the offset into the buffer
         * @return std::string_view is the view of the string at \p offset
         */
        inline std::string_view buffer_at_str(const size_t &offset) const
        {
            return std::basic_string_view{buffer_at_cstr(offset)};
        }

        /**
         * @brief Allocates the underlying buffer. Implementations must call this
         * at least once before attempting to assign to or read from the buffer.
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