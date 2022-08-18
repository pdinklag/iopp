/**
 * util/bit_unpacker.hpp
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

#ifndef _IOPP_UTIL_BIT_UNPACKER_HPP
#define _IOPP_UTIL_BIT_UNPACKER_HPP

#include <concepts>
#include <iterator>

#include "bits.hpp"
#include "pack_word.hpp"

namespace iopp {

/**
 * \brief Unpacks bits from \ref PackWord "PackWords" taken from an input iterator.
 * 
 * This class maintains a \em current pack word that bits are read from.
 * The pack word type is determined by the input iterator.
 * When the pack word runs empty, the next pack word is read using the input iterator.
 * 
 * \tparam Input the input iterator type
 */
template<std::input_iterator Input>
requires std::convertible_to<std::iter_value_t<Input>, PackWord>
class BitUnpacker {
private:
    PackWord pack_;
    size_t i_;

    Input in_;

    void advance() {
        pack_ = *in_++;
        i_ = 0;
    }

public:
    /**
     * \brief Constructs a bit unpacker
     * 
     * \param in the input iterator for packed words
     */
    BitUnpacker(Input in) : in_(in), i_(PACK_WORD_BITS) {
    }

    /**
     * \brief Reads a single bit
     * 
     * \return the value of the read bit
     */
    bool read() {
        if(i_ >= PACK_WORD_BITS) advance();
        return (bool)(pack_ & set_bit(i_++));
    }

    /**
     * \brief Reads multiple bits
     * 
     * The read bits will occupy the low bits of the returned word.
     * In case more than `PACK_WORD_BITS` bits are read, the overflow bits are lost.
     * 
     * \param num the number of bits to be read
     * \return the read bits
     */
    PackWord read(size_t num) {
        PackWord bits = 0;
        size_t j = 0;

        if(i_ >= PACK_WORD_BITS) advance();

        while(i_ + num > PACK_WORD_BITS) {
            // not all bits can be read from current pack, read as many as possible and advance
            auto const avail = PACK_WORD_BITS - i_;
            bits |= extract_low(pack_ >> i_, avail) << j;

            num -= avail;
            j += avail;
            advance();
        }

        // all remaining bits can be read from the current pack
        if(num) {
            bits |= extract_low(pack_ >> i_, num) << j;
            i_ += num;
        }

        return bits;
    }

    /**
     * \brief Returns the position of the next bit to be read in the current pack word
     * 
     * This is equivalent to the number of bits already read from the pack.
     * To retrieve the total number of read bits by this bit unpacker, add this number to the number of pack words retrieved from the input iterator.
     * Note that this class does not store the latter; it is the responsibility of the programmer to keep track of it.
     * 
     * \return size_t the position of the next bit to be read in the current pack word
     */
    size_t pack_pos() const { return i_ % PACK_WORD_BITS; }
};

}

#endif
