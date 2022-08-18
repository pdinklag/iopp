/**
 * util/output_iterator_base.hpp
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

#ifndef _IOPP_UTIL_OUTPUT_ITERATOR_BASE_HPP
#define _IOPP_UTIL_OUTPUT_ITERATOR_BASE_HPP

#include "latent_writer.hpp"

namespace iopp  {

/**
 * \brief Utility for implementing output iterators satisfying the `std::output_iterator` concept for the given item type
 * 
 * Classes can inherit from this and add only a small amount of boilerplate code to fully satisfy `std::output_iterator`.
 * For example, use the following template:
 * \code{.cpp}
 * template<typename Item>
 * class MyOutputIterator : public IOPP OutputIteratorBase<Item> {
 * private:
 *     using namespace iopp;
 *     using IteratorBase = OutputIteratorBase<Item>;
 * 
 * public:
 *     // type declarations required to satisfy std::output_iterator<Item>
 *     using IteratorBase::iterator_category;
 *     using IteratorBase::difference_type;
 *     using IteratorBase::value_type;
 *     using IteratorBase::pointer;
 *     using IteratorBase::reference;
 * 
 *     // item access
 *     using IteratorBase::operator*;
 * 
 *     // postfix incrementation
 *     auto operator++(int) { return LatentWriter<Item, MyOutputIterator<Item>>(*this); }
 * 
 *     // prefix incrementation
 *     MyOutputIterator& operator++() {
 *         auto& item = **this;
 *         // === INSERT OUTPUT IMPLEMENTATION FOR ITEM ===
 *         return *this;
 *     }
 * };
 * \endcode
 * 
 * The \ref operator* "`*` operator" provides access to the item to be output on prefix incrementation.
 * Postfix incrementation can easily be handled using the \ref LatentWriter utility like in the example above.
 * To that end, only prefix incrementation needs to be implemented.
 * 
 * \tparam Item the output item type
 */
template<typename Item>
class OutputIteratorBase {
public:
    using iterator_category = std::output_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Item;
    using pointer           = Item*;
    using reference         = Item&;

private:
    Item next_;

public:
    OutputIteratorBase() {}
    OutputIteratorBase(OutputIteratorBase&&) = default;
    OutputIteratorBase& operator=(OutputIteratorBase&&) = default;
    OutputIteratorBase(OutputIteratorBase const&) = default;
    OutputIteratorBase& operator=(OutputIteratorBase const&) = default;

    /**
     * \brief Accesses the next item
     * 
     * \return a reference to the next item
     */
    Item& operator*() {
        return next_;
    }
};

}

#endif
