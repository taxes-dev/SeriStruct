# SeriStruct IDL
To avoid writing a lot of boilerplate code for records, SeriStruct provides a script that converts a simple interface definition language (IDL) for each record into a C++ class.

You can run the script like this:

```
ssgen.py -i <input file> -o <output dir>
```

Since a single input file can contain multiple definitions, only an output directory is required to be specified. Each class will be written as a separate `.hpp` file with the same name as the class.

## Writing Record IDLs
The IDL for SeriStruct is very simple and straightforward. You can have one or more structs defined in each file, separated by one or more completely blank line. Each struct starts with a header, shown below. There can be no whitespace (spaces or tabs) at the beginning of these lines.

```
"<optional description>"
TestRecord:
    ...
```

The name of the struct must be a [valid C++ identifier](https://en.cppreference.com/w/cpp/language/identifiers) since it will be used for the name of the generated class. The optional description (must be enclosed in quotes) will be copied into a comment on the class if specified. You can repeat the optional description line multiple times for multiline comments.

After the name of the struct, one or more fields should be defined in the following format:

```
    "<optional description>"
    <field name> <data type>
```

Each line must start with at least one whitespace character (spaces or tabs). Individual fields must not have blank lines between them. After that, three columns of whitespace delimited data are specified. First the field's name (a valid C++ identifier as with the class name), then the data type (see the table of valid types below), and finally an optional description (to be used for a field comment).

| IDL data type | Corresponding C++ data type | Alignment (bytes) |
| --- | --- | --- |
| bool | bool | 1 |
| char | char | 1|
| f32 | float | 4 |
| f64 | double | 8 |
| i8 | int8_t | 1 |
| i16 | int16_t | 2 |
| i32 | int32_t | 4 |
| i64 | int64_t | 8 |
| u8 | uint8_t | 1 |
| u16 | uint16_t | 2 |
| u32 | uint32_t | 4 |
| u64 | uint64_t | 8 |
| uchar | unsigned char | 1 |
| cstr | char * | 1 |
| str | string | 1 |

Additionally, a type can have a subscript (example: `i32[3]`). This indicates a fixed array and is represented via `std::array`. A type can also be optional (example: `optional<char>`), which is represented as `std::optional`.

Note that `cstr` and `str` are not compatible with `optional`, and they must supply a maximum length using a subscript similar to an array. `cstr` requires 2 more bytes than the maximum length to account for the NUL terminator and a flag for whether the string is present or not (`nullptr`).

Here's an example of a complete record:

```
"This is a test record"
TestRecord:
    "You can put a small number here"
    small_value i16
    "You can put a bigger number here"
    big_value u64
    "Floating point number"
    fractional f32
    "A single character field"
    a_single_character char
    "Is the record a test?"
    is_test bool
    "An array of 3 integers"
    an_array i32[3]
    "An optional character"
    maybe_char optional<char>
    "This is a string with a maximum length of 50 characters"
    a_string str[50]
```

This would result in the following class:

```c++
/**
 * This is a test record
 */
class TestRecord : public Record
{
public:
    TestRecord(/*...*/) : Record{/*...*/} { /*...*/ }
    ~TestRecord() {}

    /**
     * You can put a small number here
     */
    inline int16_t small_value() const { /*...*/ }
    /**
     * You can put a bigger number here
     */
    inline uint64_t big_value() const { /*...*/ }
    /**
     * Floating point number
     */
    inline float fractional() const { /*...*/ }
    /**
     * A single character field
     */
    inline char a_single_character() const { /*...*/ }
    /**
     * Is the record a test?
     */
    inline bool is_test() const { /*...*/ }
    /**
     * An array of 3 integers
     */
    inline std::array<int32_t, 3> an_array() const { /*...*/ }
    /**
     * An optional character
     */
    inline std::optional<char> maybe_char() const { /*...*/ }
    /**
     * This is a string with a maximum length of 50 characters
     */
    inline std::string & a_string() const { /*...*/ }

private:
    /* ... */
}
```

The class would automatically include all of the boiler plate code needed for inherited functionality from `Record`.

## Desgin Considerations
To maintain serialization compatbility (forward), avoid making data type changes or field order changes to in-use fields. Putting fields at the end of the record will not impact existing data or implementations.

Part of the underlying functionality of `Record` will byte-align fields in the internal buffer according to the table shown above. Therefore, it is recommended that you put single-byte fields like `char` and `bool` next to each other to avoid excessive padding. 64-bit values are always 8-byte aligned to allow cross compatibility on 64-bit and 32-bit compilation.