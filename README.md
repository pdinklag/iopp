# io++ &ndash; I/O Utilities for C++

This header-only C++20 library provides an API for various frequently used I/O operations:

* Fast buffering [file input and output streams](#file-i/o) using the POSIX API, if available.
* [Iterators](#stream-iterators) over "STL-like" input and output streams for iterator-based I/O.
* [Bitwise I/O](#bitwise-i/o) streams powered by bit and character packing.
* [Memory-mapped files](#memory-mapped-files).
* [Miscellaneous](#miscellaneous) utility functions.

### Requirements

This library is written in C++23, a corresponding compiler is required that fully supports concepts. Tests have been done only with GCC 13. Apart from that, the library as no external dependencies. For building the [unit tests](#unit-tests), CMake is required.

Note that this library is currently only targeted at POSIX systems. I would appreciate contributions towards supporting Windows.

### License

```
MIT License

Copyright (c) Patrick Dinklage

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Unit Tests

The library comes with unit tests powered by [doctest](https://github.com/doctest/doctest).

Using CMake, you can build and run the benchmark using the following chain of commands in the repository root:

```sh
mkdir build; cd build
cmake ..
make
make test
```

## Usage

The library is header only, so all you need to do is make sure it's in your include path.

In case you use CMake, you can embed this repository into yours (e.g., as a git submodule) and add it like so:

```cmake
add_subdirectory(path/to/iopp)
```

You can then link against the `iopp` interface library, which will automatically add the include directory to your target.

### File I/O

The STL file I/O streams (`fstream`) are slow due to various reasons, one being because they use a virtual class hierarchy. Using an operating system's file I/O functionality directly typically yields much better performance, but often means using a C API.

This library provides the `FileInputStream` and `FileOutputStream` classes that are mostly STL compilant, non-virtual and use the POSIX API, if avaible to provide fast I/O performance. In case the POSIX API is not available, they simply fall back to using the STL streams. Furthermore, I/O is done buffered, which speeds up even use of the STL streams, because virtual function calls are mostly avoided.

As for "mostly" STL compilant: the `FileOutputStream` does not support seeking.

Here's a simple example:

```cpp
#include <iopp/file_input_stream.hpp>
#include <iopp/file_output_stream.hpp>

int main(int argc, char** argv) {
    if(argc < 3) return -1; // usage: [INPUT] [OUTPUT]

    // read the last 123 characters from the input file and write them to the output file
    char buf[123];
    
    {
        iopp::FileInputStream fin(argv[1]);
        fin.seekg(123, std::ios::end);
        fin.read(buf, 123);
        if(fin.gcount() == 123) {
            return 0; // success
        } else {
            return 1; // error
        }
    }
    {
        iopp::FileOutputStream fout(argv[2]);
        fout.write(buf, 123);
    }
}
```

The `FileInputStream` provides `begin` and `end` iterators so file inputs can be iterated over using ranged for loops, see below for an example.

### Stream Iterators

The STL stream API comes with stream iterators. However, it only supports streams from the STL stream class tree.

This library introduces the concepts `STLInputStreamLike` and `STLOutputStreamLike` and provides a `StreamInputIterator` and `StreamOutputIterator` that can work with any stream that satisfies those concepts, including the STL stream as well as the file streams introduced above.

Here's a simple example:

```cpp
#include <iopp/file_input_stream.hpp>
#include <iopp/file_output_stream.hpp>
#include <iopp/stream_output_iterator.hpp>

int main(int argc, char** argv) {
    if(argc < 3) return -1; // usage: [INPUT] [OUTPUT]

    // perform a byte-wise copy of the input file to the output file
    iopp::FileInputStream fin(argv[1]);
    iopp::FileOutputStream fout(argv[2]);

    iopp::StreamOutputIterator out(fout);
    for(char const c : fin) *out++ = c;
    // alternatively: std::copy(fin.begin(), fin.end(), iopp::StreamOutputIterator(fout));
    
    return 0;
}
```

### Bitwise I/O

The library provides an API for bitwise reading and writing from or to STL-like streams.

Internally, this works by packing bits in a buffer word. This is backed by the type `uintmax_t`, which is typically 64 bits on nowadays' hardware. Note that output is aligned to that types' width, and you may therefore experience an overhead of up to 63 bits in your output.

Here's a simple example:

```cpp
#include <iopp/bitwise_io.hpp>

int main(int argc, char** argv) {
    if(argc < 2) return -1; // usage: [OUTPUT]
    
    // do some bitwise output to the output file
    {
        iopp::FileOutputStream fout(argv[1]);
        auto bits = iopp::bitwise_output_to(fout);
        bits.write(0); // write a 0-bit
        bits.write(1); // write a 1-bit
        bits.write(0b1011, 4); // write the 4 lowest bits of the given word
    }
    
    // and now read it back bitwise
    bool ok = true;
    {
        iopp::FileInputStream fin(argv[1]);
        auto bits = iopp::bitwise_input_from(fin);
        
        bool const b1 = bits.read();  // read a single bit
        bool const b2 = bits.read();  // read a single bit
        auto const n  = bits.read(4); // read 4 bits into the low bits of the returned word
        
        ok = (b1 == 0 && b2 == 1 && n == 0b1011);
    }
    
    return ok ? 0 : 1;
}
```

#### Finalizers

In the above example, there was no notion of an "end of file" (EOF). In many cases, however, it is required to find out whether all bits have been read from the input. For this, the library supports *finalizers*.

When destroyed, the object returned by `bitwise_output_to` (an `iopp::BitPacker`) appends a finalizer to the bitwise output. It encodes the number of actually used bits in the final pack. To make this information available to a decoder, use the overload of `bitwise_input_from` that accepts a beginning and end iterator like so:

```cpp
iopp::FileInputStream fin(argv[1]);
auto bits = iopp::bitwise_input_from(fin.begin(), fin.end());
size_t num_bits_read = 0;
while(bits) {
    bits.read();
    ++num_bits_read;
}
```

Note that finalizers, in the worst case, may add an additional pack to the output such that up to 58 bits are wasted (on a 64-bit architecture). If not needed, finalizers can be disabled by passing `false` as the second parameter to `bitwise_output_to`.

#### Concepts

The library comes with two concepts: `iopp::BitSink` and `iopp::BitSource`. These reflect the API of the objects returned by `bitwise_input_from` and `bitwise_output_to`, respectively, and allow for stating proper C++20 requirements for template parameters.

### Memory-Mapped Files

The library provides an API to memory-map files. Naturally, it only works if the operating system supports this.

Here's a simple example:

```cpp
#include <algorithm>
#include <iopp/file_output_stream.hpp>
#include <iopp/memory_mapped_file.hpp>
#include <iopp/stream_output_iterator.hpp>

int main(int argc, char** argv) {
    if(argc < 3) return -1; // usage: [OUTPUT]
    
    if(iopp::MemoryMappedFile::available()) { // test if we can do this on this OS
        // memory-map the input file
        iopp::MemoryMappedFile mmap(argv[1]);
        char const* bytes = (char const*)mmap.data();

        // perform a byte-wise copy to the output
        iopp::FileOutputStream fout(argv[2]);
        std::copy(bytes, bytes + mmap.size(), iopp::StreamOutputIterator(fout));

        return 0;
    } else {
        return -1;
    }
}
```

### Miscellaneous

The library provides a few more utilities:

* The concept `iopp::InputIterator<Item>` allows you to constrain a type to an `std::input_iterator` whose `std::iter_value_t` is `std::convertible_to<Item>`. This can save a few `requires` statements in your declarations:

  ```cpp
  template<std::input_iterator It> requires std::convertible_to<std::iter_value_t<It>, Item>
  ```

  becomes:

  ```cpp
  template<iopp::InputIterator<Item> It>
  ```

* Using `iopp::stdin_is_pipe()` (in `iopp/stdin.hpp`), you can quickly test whether is something on the standard input.

* If you just need a file to be loaded as a string, use `iopp::load_file_str` (in `iopp/load_file.hpp`).

* Ever need to write an output iterator that satisfies the `std::output_iterator` concept? Base it off `iopp::OutputIteratorBase` (in `iopp/util/output_iterator_base.hpp`)!

