/**
 * util/char_packer.hpp
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

#ifndef _IOPP_UTIL_CHAR_PACKER_HPP
#define _IOPP_UTIL_CHAR_PACKER_HPP

#include <bit>

#include "dead_end.hpp"
#include "pack_word.hpp"
#include "../concepts.hpp"

namespace iopp {

/**
 * \brief Input iterator packing characters into \ref PackWord "PackWords"
 * 
 * When a word is requested via this iterator, the corresponding number (determined by the \ref PackWord "pack word type") of characters is read from the source input iterator.
 * Once the source iterator reaches the end, the packer will also enter an \em end state.
 * 
 * The following is a usage example:
 * \code{.cpp}
 * std::string s; // some input string
 * std::vector<PackWord> v;
 * 
 * // pack characters from s into words and push them into v
 * std::copy(CharPacker(s.begin(), s.end()), {}, std::back_inserter(v));
 * \endcode
 * 
 * This class satisfies the `std::input_iterator` concept.
 * 
 * \tparam CharInputIterator the source iterator type
 */
template<InputIterator<char> CharInputIterator>
class CharPacker {
private:
    CharInputIterator in_, end_;

    PackWord current_;
    bool reached_end_;

    void advance() {
        reached_end_ = (in_ == end_);
        if(!reached_end_) {
            constexpr size_t chars_per_int = sizeof(PackWord);
            if constexpr(std::endian::native == std::endian::little) {
                char* p = (char*)&current_ + chars_per_int - 1;
                for(size_t i = 0; i < chars_per_int; i++) {
                    *p-- = *in_++;
                }
            } else {
                char* p = (char*)&current_;
                for(size_t i = 0; i < chars_per_int; i++) {
                    *p++ = *in_++;
                }
            }
        }
    }

public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = PackWord;
    using pointer           = PackWord*;
    using reference         = PackWord&;

    /**
     * \brief Constructs a character packer and immediately reads the initial word from it
     * 
     * \param in the source iterator
     * \param end the end input iterator
     */
    CharPacker(CharInputIterator in, CharInputIterator end) : in_(in), end_(end) {
        advance();
    }

    /**
     * \brief Constructs an invalid character packer, which can serve as an \em end iterator
     * 
     */
    CharPacker() : in_({}), end_({}), reached_end_(true) {}
    
    CharPacker(CharPacker const&) = default;
    CharPacker(CharPacker&&) = default;
    CharPacker& operator=(CharPacker const&) = default;
    CharPacker& operator=(CharPacker&&) = default;
 
    /**
     * \brief Equality comparison
     * 
     * \param other the compared iterator
     * \return true if both iterators are in \em end state or both iterators are using the same input iterator
     * \return false otherwise
     */
    bool operator==(CharPacker const& other) const {
        // iterators that reached EOF are considered equal
        return (reached_end_ && other.reached_end_) || in_ == other.in_;
    }

    /**
     * \brief Inequality comparison
     * 
     * \param other the compared iterator
     * \return negation of \ref operator==
     */
    bool operator!=(CharPacker const&  other) const {
        return !(*this == other);
    }

     /**
     * \brief Accesses the current pack word
     * 
     * \return a const reference to the current pack word
     */
    PackWord const& operator*() const { return current_; }

    /**
     * \brief Advances the iterator
     * 
     * \return a reference to this character packer
     */
    CharPacker& operator++() {
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
