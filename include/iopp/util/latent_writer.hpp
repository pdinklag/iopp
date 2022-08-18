/**
 * util/latent_writer.hpp
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

#ifndef _IOPP_UTIL_LATENT_WRITER_HPP
#define _IOPP_UTIL_LATENT_WRITER_HPP

#include <iterator>

namespace iopp {

/**
 * \brief Utility for implementing postfix incrementation of output iterators
 * 
 * This class provides only the `*` operator that allows for writing an item.
 * Upon destruction, it (postfix) increments the targeted iterator.
 * 
 * Please refer to the documentation of \ref OutputIteratorBase for details on using this class.
 * 
 * \tparam Item the iterator item type
 * \tparam Iterator the targeted iterator type
 */
template<typename Item, std::indirectly_writable<Item> Iterator>
class LatentWriter {
private:
    Iterator* _it;

    class WriteAccessor {
    private:
        LatentWriter* writer_;

    public:
        WriteAccessor(LatentWriter& writer) : writer_(&writer) {
        }

        WriteAccessor() = delete;
        WriteAccessor(WriteAccessor&&) = delete;
        WriteAccessor(WriteAccessor const&) = delete;
        WriteAccessor& operator=(WriteAccessor&&) = delete;
        WriteAccessor& operator=(WriteAccessor const&) = delete;

        WriteAccessor& operator=(Item&& item) {
            *(*writer_->_it) = std::move(item);
            return *this;
        }

        WriteAccessor& operator=(Item const& item) {
            (*this) = Item(item);
            return *this;
        }
    };

public:
    /**
     * \brief Constructs a latent writer for the specified target iterator
     * 
     * \param target the target iterator
     */
    LatentWriter(Iterator& target) : _it(&target) {
    }

    LatentWriter() = delete;
    LatentWriter(LatentWriter&&) = delete;
    LatentWriter(LatentWriter const&) = delete;
    LatentWriter& operator=(LatentWriter&&) = delete;
    LatentWriter& operator=(LatentWriter const&) = delete;

    /**
     * \brief Post-increments the target iterator
     * 
     */
    ~LatentWriter() {
        ++(*_it);
    }

    /**
     * \brief Returns a write accessor to the target iterator
     * 
     * \return a write accessor to the target iterator
     */
    WriteAccessor operator*() {
        return WriteAccessor(*this);
    }
};

}

#endif
