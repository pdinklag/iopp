/**
 * concepts.hpp
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

#ifndef _IOPP_CONCEPTS_HPP
#define _IOPP_CONCEPTS_HPP

#include <concepts>
#include <iostream>
#include <iterator>

namespace iopp {

/**
 * \brief Concept for types that have a basic `std::istream`-like interface
 * 
 * In order to satisfy the concept, the type must
 * * provide `char_type` that determines the read item type
 * * be explictly convertible to `bool` to test if the stream is still \em good ,
 * * define the functions `get` and `read` to read characters and
 * * support the queries `gcount` (number of characters successfully read) and `tellg` (current position in the stream).
 *
 * \tparam T the type
 */
template<typename T>
concept STLInputStreamLike =
    requires {
        typename T::char_type;
    } &&
    requires(const T subject) {
        { subject.good() } -> std::same_as<bool>; // good()
        { (bool)subject }; // explicit bool conversion
        { subject.gcount() } -> std::convertible_to<size_t>; // gcount (get number of bytes read)
    } &&
    requires(T subject) {
        { subject.get() } -> std::convertible_to<typename T::char_type>; // get (next character from stream)
        { subject.tellg() } -> std::convertible_to<size_t>; // tellg (current stream position) -- actually not const in std::istream
    } &&
    requires(T subject, typename T::char_type* outp, size_t len) {
        { subject.read(outp, len) }; // read -- we don't care about the return type, which is typically a reference to some abstract base type
    };

/**
 * \brief Concept for types that have a basic `std::ostream`-like interface
 * 
 * In order to satisfy the concept, the type must
 * * provide `char_type` that determines the written item type
 * * define the functions `put` and `write` to write characters
 * * define the function `flush` that immediately writes any buffered characters to the underlying sink
 * * support the query `tellp` (current position in the stream).
 *
 * Note that the `<<` operator does \em not have to be supported.
 * 
 * \tparam T the type
 */
template<typename T>
concept STLOutputStreamLike =
    requires {
        typename T::char_type;
    } &&
    requires(T subject) {
        { subject.flush() }; // flush -- we don't care about the return type, which is typically a reference to some abstract base type
        { subject.tellp() } -> std::convertible_to<size_t>; // tellp (current stream position) -- actually not const in std::ostream
    } &&
    requires(T subject, typename T::char_type c) {
        { subject.put(c) }; // put (one character on stream) -- we don't care about the return type, which is typically a reference to some abstract base type
    } &&
    requires(T subject, typename T::char_type const* inp, size_t len) {
        { subject.write(inp, len) }; // write -- we don't care about the return type, which is typically a reference to some abstract base type
    };

}

#endif
