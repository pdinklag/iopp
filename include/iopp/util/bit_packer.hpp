/**
 * util/bit_packer.hpp
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

#ifndef _IOPP_UTIL_BIT_PACKER_HPP
#define _IOPP_UTIL_BIT_PACKER_HPP

#include <cstdint>
#include <iterator>
#include <limits>
#include <utility>

#include "bits.hpp"
#include "pack_word.hpp"

namespace iopp {

/**
 * \brief Packs bits into \ref PackWord "PackWords" and emits them to an output iterator.
 * 
 * This class maintains a \em current pack word that bits are written to.
 * When the pack word is full, \ref flush is called or the packer is destroyed, the word is forwarded to the specified output iterator.
 * 
 * \tparam Pack the pack integer type
 * \tparam Output the output iterator type
 */
template<std::output_iterator<PackWord> Output>
class BitPacker {
private:
    PackWord pack_;
    size_t i_;

    Output out_;

    void reset() {
        pack_ = 0U;
        i_ = 0;
    }

public:
    /**
     * \brief Constructs a bit packer
     * 
     * \param out the output iterator
     */
    BitPacker(Output out) : out_(out) {
        reset();
    }

    /**
     * \brief Calls \ref flush
     * 
     */
    ~BitPacker() {
        flush();
    }

    /**
     * \brief Writes a single bit to the output
     * 
     * \param bit the bit to write
     */
    void write(bool const bit) {
        // write single bit
        pack_ |= set_bit(i_) & (-bit); // nb: branchless ternary version of (bit ? set_bit(i_) : 0)

        // potentially flush
        if(++i_ >= PACK_WORD_BITS) flush();
    }

    /**
     * \brief Writes multiple bits to the output
     * 
     * The bits are given as the low bits of an unsigned integer.
     * Any bits beyond the `PACK_WORD_BITS` bits of the input integer will be clear.
     * 
     * \param bits the unsigned integer containing the bits to be written
     * \param num the number of low bits from `bits` to be written
     */
    void write(uintmax_t bits, size_t num) {
        // write num low bits (in MSBF order)
        while(i_ + num > PACK_WORD_BITS) {
            // not all bits fit into current pack, write as many as possible and advance
            auto const fit = PACK_WORD_BITS - i_;
            pack_ |= extract_low(bits, fit) << i_;

            i_ = PACK_WORD_BITS;
            flush();
            bits >>= fit; // nb: safe, because fit < 64
            num -= fit;
        }

        // all remaining bits completely fit into the current pack
        if(num) {
            pack_ |= extract_low(bits, num) << i_;
            i_ += num;
            if(i_ >= PACK_WORD_BITS) flush();
        }
    }

    /**
     * \brief Flushes the current bit pack to the output
     * 
     * Any unwritten bits in the pack are set to zero.
     */
    void flush() {
        if(i_) {
            *out_++ = pack_;
            reset();
        }
    }

    /**
     * \brief Returns the position of the next bit to be written in the current pack word
     * 
     * This is equivalent to the number of bits already written to the pack.
     * To retrieve the total number of written bits by this bit packer, add this number to the number of pack words emitted to the output iterator.
     * Note that this class does not store the latter; it is the responsibility of the programmer to keep track of it.
     * 
     * \return size_t the position of the next bit to be written in the current pack word
     */
    size_t pack_pos() const { return i_ % PACK_WORD_BITS; }
};

}

#endif
