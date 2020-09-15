# SeriStruct
Lightweight C++ struct serializer inspired by a couple of CppCon 2020 talks, specifically Bjarne Stroustrup's "The Beauty and Power of Primitive C++" and Phil Nash's "Test Driven C++". Specifically I wanted to try my hand at writing something similar to Stroustrup's "Flat" struct transport with accompanying unit tests as part of a coding "kata" (as Nash called it).

## Requirements
* CMake 3.16 or later
* Modern C++ compiler (C++17)

## Project Structure
* `src/lib` - Contains the library and its header folder.
* `src/tests` - Contains Catch2 unit test harness.

Unit tests are run after every build.

## License
Distributed under an [MIT license](LICENSE.md). You're welcome to copy, modify, and distribute, so long as you follow the rules of the license.