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

#include <bit>
#include <cassert>
#include <cstddef>
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
 * This class satisfies the \ref iopp::BitSink concept.
 * 
 * \tparam Pack the pack integer type
 * \tparam Output the output iterator type
 */
template<std::output_iterator<PackWord> Output>
class BitPacker {
private:
    static constexpr size_t FINALIZER_BITS = std::bit_width(PACK_WORD_BITS - 1);
    static constexpr size_t PAYLOAD_BITS = PACK_WORD_BITS - FINALIZER_BITS;
    static constexpr size_t FINALIZER_LSH = PAYLOAD_BITS - 1;

    static constexpr PackWord encode_finalizer(size_t const finalizer) {
        return (PackWord(finalizer) - 1) << FINALIZER_LSH;
    }

    PackWord pack_;
    size_t i_;
    size_t num_bits_written_;

    Output out_;
    bool finalize_;
    bool was_ever_flushed_;

    void reset() {
        pack_ = 0U;
        i_ = 0;
    }

public:
    /**
     * \brief Constructs a bit packer
     * 
     * \param out the output iterator
     * \param finalize if true, when destroying the bit packer, it will emit a final piece of information for a future \ref BitUnpacker to detect the end of the bit stream
     */
    BitPacker(Output out, bool const finalize = true) : out_(out), num_bits_written_(0), finalize_(finalize), was_ever_flushed_(false) {
        reset();
    }

    /**
     * \brief Potentially writes finalization information and finally calls \ref flush
     * 
     */
    ~BitPacker() {
        bool const non_empty = (was_ever_flushed_ || i_ > 0); // nb: we don't write a finalizer if the output stream is empty
        if(finalize_ && non_empty) {
            PackWord const finalizer = encode_finalizer(i_);
            if(i_ >= PAYLOAD_BITS) {
                // finalization info no longer fits into this pack word, flush and write it to the next
                flush();
            }

            pack_ |= finalizer;
            i_ = PACK_WORD_BITS; // nb: make sure the final flush will do something even if this is a new (final) word
        }
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
        ++num_bits_written_;

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
        assert(num > 0);
        auto const in_num = num;

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

        num_bits_written_ += in_num;
    }

    /**
     * \brief Flushes the current bit pack to the output
     * 
     * Any unwritten bits in the pack are set to zero.
     */
    void flush() {
        if(i_) {
            was_ever_flushed_ = true;

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
     * \return the position of the next bit to be written in the current pack word
     */
    size_t pack_pos() const { return i_ % PACK_WORD_BITS; }

    /**
     * \brief Reports the number of bits written since instantiation.
     * 
     * This does not include the finalizer, if any.
     * 
     * \return the number of bits written since instantiation
     */
    size_t num_bits_written() const { return num_bits_written_; }
};

}

#endif
