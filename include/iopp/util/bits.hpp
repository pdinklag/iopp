/**
 * util/bits.hpp
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

#ifndef _IOPP_UTIL_BITS_HPP
#define _IOPP_UTIL_BITS_HPP

#include <cstddef>
#include <cstdint>

namespace iopp {

/**
 * \brief Constructs an unsigned integer where the `i`-th bit is set and all other bits are clear
 * 
 * \param i the index of the set bit; for values of 64 or above, the result is undefined
 * \return an unsigned integer where the `i`-th bit is set and all other bits are clear
 */
constexpr inline uintmax_t set_bit(size_t const i) {
    return uintmax_t(1) << i;
}

/**
 * \brief Computes a bit mask for the extraction of the given number of low bits
 * 
 * \param num the number of low bits to extract; for values outside of the interval `(0, 64]`, the result is undefined
 * \return an unsigned integer with the \c num lowest bits set to one and the high bits set to zero
 */
constexpr inline uintmax_t low_mask(size_t const num) {
    return ~((UINTMAX_MAX << (num - 1)) << 1);
}

/**
 * \brief Extracts low bits from the given integer
 * 
 * \param v the integer to extract bits from
 * \param num the number of low bits to extract; for values outside of the interval `(0, 64]`, the result is undefined
 * \return an unsigned integer containing the `num` low bits of `v` with the remaining high bits set to zero
 */
constexpr inline uintmax_t extract_low(uintmax_t const v, size_t const num) {
    // nb: we expect num to be in (0, 64]
    return v & low_mask(num);
}

}

#endif
