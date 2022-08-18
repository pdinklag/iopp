/**
 * util/dead_end.hpp
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

#ifndef _IOPP_UTIL_DEAD_END_HPP
#define _IOPP_UTIL_DEAD_END_HPP

namespace iopp {

/**
 * \brief Wrapper marking a dead end of an input iterator
 * 
 * This is typically returned by the postfix increment operator of tudocomp's input iterators.
 * It serves no functionality other than providing access to the previous element via \ref operator* .
 * 
 * \tparam Item the item type
 */
template<typename Item>
class DeadEnd {
private:
    Item item_;

public:
    /**
     * \brief Constructs a dead end
     * 
     * \param item the wrapped item
     */
    DeadEnd(Item&& item) : item_(std::move(item)) {
    }

    /**
     * \brief Accesses the wrapped item
     * 
     * \return a const reference to the wrapped item
     */
    Item const& operator*() const {
        return item_;
    }
};

}

#endif
