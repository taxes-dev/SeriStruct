# SeriStruct
Lightweight C++ struct serializer inspired by a couple of CppCon 2020 talks, specifically Bjarne Stroustrup's "The Beauty and Power of Primitive C++" and Phil Nash's "Test Driven C++". Specifically I wanted to try my hand at writing something similar to Stroustrup's "Flat" struct transport with accompanying unit tests as part of a coding "kata" (as Nash called it).

## Requirements
* CMake 3.16 or later
* Modern C++ compiler (C++17)
* Python 3.x

## Project Structure
* `src/idl` - Scripts for generating SeriStruct records from IDL.
* `src/lib` - The library and its header.
* `src/tests` - Catch2 unit test harness.

Unit tests are run after every build.

## Limitations
* The code assumes 32-bit floats and 64-bit doubles. There is a unit test that will warn if this is not the case.
* The underlying data is technically not portable due to endianess (if there is a mismatch between a system serializing and a system deserializing, you will get wrong data). For speed reasons there is no check to swap bytes, which is left as a concern of a higher-level transport mechanism.

## License
Distributed under an [MIT license](LICENSE.md). You're welcome to copy, modify, and distribute, so long as you follow the rules of the license.