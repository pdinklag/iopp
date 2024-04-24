/**
 * steam_output_iterator.hpp
 * part of pdinklag/iopp
 * 
 * MIT License
 * 
 * Copyright (c) Patrick Dinklage
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

#ifndef _IOPP_STREAM_OUTPUT_ITERATOR_HPP
#define _IOPP_STREAM_OUTPUT_ITERATOR_HPP

#include "concepts.hpp"

namespace iopp {

/**
 * \brief Output iterator for output streams
 * 
 * This iterator writes any character it is assigned to (dereferenced or not) to the given output stream via the `put` function.
 * Incrementing an output stream iterator is a no-op.
 * 
 * Other than the standard library input stream iterators, this iterator has the weaker constraint where the stream has to be only \ref iopp::STLOutputStreamLike "STLOutputStreamLike".
 * This enables use of input streams that share a similar interface, but don't rely on virtual inheritance at their core.
 * 
 * This iterator satisfies the \c std::output_iterator concept for the output stream's character type.
 * 
 * \tparam OutputStream the output stream type
 */
template<STLOutputStreamLike OutputStream>
class StreamOutputIterator {
private:
    using Char = OutputStream::char_type;

    OutputStream* stream_;

public:
    using iterator_category = std::output_iterator_tag;
    using difference_type = ssize_t;
    using value_type = void;
    using pointer = void;
    using reference = void;

    /**
     * \brief Constructs an iterator for the given stream
     * 
     * \param stream the stream to write to
     */
    StreamOutputIterator(OutputStream& stream) : stream_(&stream) {}

    StreamOutputIterator() : stream_(nullptr) {}
    StreamOutputIterator(StreamOutputIterator const&) = default;
    StreamOutputIterator(StreamOutputIterator&&) = default;
    StreamOutputIterator& operator=(StreamOutputIterator const&) = default;
    StreamOutputIterator& operator=(StreamOutputIterator&&) = default;

    void operator=(Char c) { stream_->put(c); }

    StreamOutputIterator& operator*() { return *this; }
    StreamOutputIterator& operator++(int) { return *this; }
    StreamOutputIterator& operator++() { return *this; }
};

}

#endif
