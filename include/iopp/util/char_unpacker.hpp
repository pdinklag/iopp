/**
 * util/char_unpacker.hpp
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

#ifndef _IOPP_UTIL_CHAR_UNPACKER_HPP
#define _IOPP_UTIL_CHAR_UNPACKER_HPP

#include <bit>

#include "output_iterator_base.hpp"
#include "pack_word.hpp"

namespace iopp {

/**
 * \brief Output iterator unpacking characters from \ref PackWord "PackWords"
 * 
 * Words written via this iterator are unpacked into characters, which are then forwarded to the target iterator.
 * The number of characters that are extracted from one integer is determined by the \ref PackWord "pack word type".
 * 
 * This class satisfies the `std::output_iterator` concept for \ref PackWord .
 * 
 * \tparam CharOutputIterator the target iterator
 */
template<std::output_iterator<char> CharOutputIterator>
class CharUnpacker : public OutputIteratorBase<PackWord> {
private:
    using IteratorBase = OutputIteratorBase<PackWord>;

    CharOutputIterator out_;

public:
    // type declarations required to satisfy std::output_iterator<Item>
    using IteratorBase::iterator_category;
    using IteratorBase::difference_type;
    using IteratorBase::value_type;
    using IteratorBase::pointer;
    using IteratorBase::reference;

    /**
     * \brief Constructs a character unpacker
     * 
     * \param out the target iterator
     */
    CharUnpacker(CharOutputIterator out) : out_(out) {}

    /**
     * \brief Constructs an invalid character unpacker
     * 
     */
    CharUnpacker() : out_({}) {}
    
    CharUnpacker(CharUnpacker const&) = default;
    CharUnpacker(CharUnpacker&&) = default;
    CharUnpacker& operator=(CharUnpacker const&) = default;
    CharUnpacker& operator=(CharUnpacker&&) = default;
 
    using IteratorBase::operator*;
 
    auto operator++(int) { return LatentWriter<PackWord, std::remove_pointer_t<decltype(this)>>(*this); }
 
    CharUnpacker& operator++() {
        auto& item = **this;
        
        constexpr size_t chars_per_int = sizeof(PackWord);
        if constexpr(std::endian::native == std::endian::little) {
            char* p = (char*)&item + chars_per_int - 1;
            for(size_t i = 0; i < chars_per_int; i++) {
                *out_++ = *p--;
            }
        } else {
            char* p = (char*)&item;
            for(size_t i = 0; i < chars_per_int; i++) {
                *out_++ = *p++;
            }
        }

        return *this;
    }
};

}

#endif
