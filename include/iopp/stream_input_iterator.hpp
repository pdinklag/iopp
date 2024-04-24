/**
 * stream_input_iterator.hpp
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

#ifndef _IOPP_STREAM_INPUT_ITERATOR_HPP
#define _IOPP_STREAM_INPUT_ITERATOR_HPP

#include <concepts>
#include <iterator>
#include <memory>
#include <utility>

#include "concepts.hpp"
#include "util/dead_end.hpp"

namespace iopp {

/**
 * \brief Input iterator for input streams
 * 
 * Items requested from this iterator are streamed using the linked stream's `get` function.
 * 
 * Other than the standard library input stream iterators, this iterator has the weaker constraint where the stream has to be only \ref iopp::STLInputStreamLike "STLInputStreamLike".
 * This enables use of input streams that share a similar interface, but don't rely on virtual inheritance at their core.
 * 
 * Once the stream reports EOF, the iterator will go in a EOF state where it is considered equal to any other input stream iterator in EOF state.
 * Especially, this means that iterators in EOF state will be considered equal to default-constructed iterators, which can thus be used as \em end iterators.
 * 
 * This iterator satisfies the \c std::input_iterator concept.
 * 
 * \tparam InputStream the input stream type
 */
template<STLInputStreamLike InputStream>
class StreamInputIterator {
public:
    /**
     * \brief The character type as given by the input stream
     * 
     */
    using Char = typename InputStream::char_type;

private:
    InputStream* stream_;

    bool eof_;
    Char current_;

    void advance() {
        current_ = stream_->get();
        if(!*stream_) {
            // EOF
            eof_ = true;
            current_ = {};
        } else {
            eof_ = false;
        }
    }

    StreamInputIterator(InputStream& stream, bool eof) : stream_(&stream), eof_(eof), current_() {
    }

public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Char;
    using pointer           = Char*;
    using reference         = Char&;

    /**
     * \brief Returns an iterator marking the end of the given stream
     * 
     * \param stream the stream the end of which is marked
     */
    static StreamInputIterator end(InputStream& stream) { return StreamInputIterator(stream, true); }

    /**
     * \brief Constructs an invalid iterator
     */
    StreamInputIterator() : stream_(nullptr), eof_(false) {
    }

    /**
     * \brief Constructs an iterator for the given stream
     * 
     * \param stream the stream to iterate over
     */
    StreamInputIterator(InputStream& stream) : stream_(&stream) {
        advance();
    }

    StreamInputIterator(StreamInputIterator&&) = default;
    StreamInputIterator& operator=(StreamInputIterator&&) = default;
    StreamInputIterator(StreamInputIterator const&) = default;
    StreamInputIterator& operator=(StreamInputIterator const&) = default;

    /**
     * \brief Equality comparison
     * 
     * \param other the compared iterator
     * \return true if both iterators are in EOF state or both iterators are using the same stream
     * \return false otherwise
     */
    bool operator==(StreamInputIterator const& other) const {
        return stream_ == other.stream_ && eof_ == other.eof_;
    }

    /**
     * \brief Inequality comparison
     * 
     * \param other the compared iterator
     * \return negation of \ref operator==
     */
    bool operator!=(StreamInputIterator const&  other) const {
        return !(*this == other);
    }

    /**
     * \brief Accesses the current item
     * 
     * \return a const reference to the current item
     */
    Char const& operator*() const {
        return current_;
    }

    /**
     * \brief Advances the iterator
     * 
     * If the underlying stream reports EOF, the iterator will enter EOF state.
     * 
     * \return a reference to this iterator
     */
    StreamInputIterator& operator++() {
        advance();
        return *this;
    }

    /**
     * \brief Advances the iterator
     * 
     * If the underlying stream reports EOF, the iterator will enter EOF state.
     * 
     * \return a dead end wrapper around the current item before advancing
     */
    auto operator++(int) {
        auto dead_end = DeadEnd(std::move(current_));
        advance();
        return dead_end;
    }
};

}

#endif
