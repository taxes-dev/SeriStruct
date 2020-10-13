# SeriStruct
Lightweight C++ struct serializer inspired by a couple of CppCon 2020 talks, specifically Bjarne Stroustrup's "The Beauty and Power of Primitive C++" and Phil Nash's "Test Driven C++". Specifically I wanted to try my hand at writing something similar to Stroustrup's "Flat" struct transport with accompanying unit tests as part of a coding "kata" (as Nash called it).

Features:
* Automated generation of record structs (see [IDL README](src/idl/README.md)).
* Support for basic data types, strings, `std::array`, and `std::optional`
* Data is only copied on assignment into or copying out of the record. Only a single allocation is made at construction.
* Optional mutability.
* Copying to/from byte buffers.
* Writing to/reading from streams.

## Requirements
* CMake 3.16 or later
* Modern C++ compiler (C++17)
* Python 3.x

## Project Structure
* `src/idl` - Scripts for generating SeriStruct records from IDL.
* `src/lib` - The library and its header.
* `src/tests` - Catch2 unit test harness.

Unit tests are run after every build.

## Supported data types

Full support for:
* All integrals (including `char`/`unsigned char`)
* `float`/`double`
* Fixed-length arrays of the above (`std::array`)
* Optional of the above (`std::optional`)

Limited support for:
* C strings (`char *`)
* C++ strings (`std::string`/`std::string_view`)

Strings are limited to a specified maximum length and always occupy that maximum length.

## Limitations
* The code assumes 32-bit floats and 64-bit doubles. There is a unit test that will warn if this is not the case.
* The underlying data is technically not portable due to endianess (if there is a mismatch between a system serializing and a system deserializing, you will get wrong data). For speed reasons there is no check to swap bytes, which is left as a concern of a higher-level transport mechanism.
* Only data structures with a size known at compile time are supported.

## License
Distributed under an [MIT license](LICENSE.md). You're welcome to copy, modify, and distribute, so long as you follow the rules of the license.