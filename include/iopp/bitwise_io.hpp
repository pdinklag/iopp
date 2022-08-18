/**
 * bitwise_io.hpp
 * part of pdinklag/iopp
 * 
 * MIT License
 * 
 * Copyright (c) 2022 Patrick Dinklage
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _IOPP_BITWISE_IO_HPP
#define _IOPP_BITWISE_IO_HPP

#include "concepts.hpp"

#include "stream_input_iterator.hpp"
#include "stream_output_iterator.hpp"

#include "util/bit_packer.hpp"
#include "util/bit_unpacker.hpp"
#include "util/char_packer.hpp"
#include "util/char_unpacker.hpp"

namespace iopp {

/**
 * \brief Constructs a \ref iopp::BitSource "BitSource" from the given input stream
 * 
 * This is shorthand for the following snippet:
 * \code{.cpp}
 * BitUnpacker(CharPacker(StreamInputIterator(in), {}))
 * \endcode
 * 
 * Note that there is no indication as to when the input ends;
 * it is the responsibility of the programmer to stop reading before the end of the stream.
 * 
 * \tparam InputStream the input stream type
 * \param in the input stream
 * \return a bit source reading from the input stream
 */
template<STLInputStreamLike InputStream>
auto bitwise_input_from(InputStream& in) {
    return BitUnpacker(CharPacker(StreamInputIterator(in), {}));
}

/**
 * \brief Constructs a \ref iopp::BitSink "BitSink" to the given output stream
 * 
 * This is shorthand for the following snippet:
 * \code{.cpp}
 * BitPacker(CharUnpacker(StreamOutputIterator(in), {}))
 * \endcode
 * 
 * \tparam OutputStream the output stream type
 * \param out the output stream
 * \return a bit sink writing to the output stream
 */
template<STLOutputStreamLike OutputStream>
auto bitwise_output_to(OutputStream& out) {
    return BitPacker(CharUnpacker(StreamOutputIterator(out)));
}

}

#endif
