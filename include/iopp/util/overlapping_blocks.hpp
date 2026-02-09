/**
 * overlapping_blocks.hpp
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

#ifndef _IOPP_OVERLAPPING_BLOCKS_HPP
#define _IOPP_OVERLAPPING_BLOCKS_HPP

#include <cstddef>
#include <memory>

#include "../concepts.hpp"

namespace iopp {

/**
 * \brief Utility for processing an input stream blockwise with a given overlap .
 * 
 * When processing a block, it is allowed to access "negative" position that pertains to the previous block.
 * This can be useful in some scenarios such as sliding windows.
 * 
 * \tparam InputStream the input stream type
 */
template<STLInputStreamLike InputStream>
class OverlappingBlocks {
private:
    using Char = typename InputStream::char_type;
    static constexpr auto EOF_TOKEN = std::char_traits<Char>::eof();

    size_t block_size_;
    size_t overlap_;

    std::unique_ptr<Char[]> buffer_;
    InputStream::int_type probe_;
    size_t cur_size_;
    size_t cur_offs_;
    Char* cur_begin_;

    InputStream* stream_;

    void read_next() {
        if(first()) {
            stream_->read(cur_begin_, block_size_);
            cur_size_ = stream_->gcount();
        } else {
            *cur_begin_ = Char(probe_);
            stream_->read(cur_begin_ + 1, block_size_ - 1);
            cur_size_ = stream_->gcount() + 1;
        }
        probe_ = stream_->get();
    }

public:
    /**
     * \brief Constructs an empty blockwise processor.
     */
    OverlappingBlocks() : block_size_(0), overlap_(0), buffer_(), probe_(EOF_TOKEN), cur_begin_(nullptr), cur_size_(0), cur_offs_(0), stream_(nullptr) {
    }

    /**
     * \brief Constructs a blockwise processor with given overlap, but without an input stream.
     * 
     * \param block_size the size of an input block
     * \param overlap the overlap size
     */
    OverlappingBlocks(size_t const block_size, size_t const overlap) : block_size_(block_size),
        overlap_(overlap),
        buffer_(std::make_unique<Char[]>(block_size_ + overlap_)),
        probe_(EOF_TOKEN),
        cur_begin_(buffer_.get() + overlap_),
        cur_size_(0),
        cur_offs_(0),
        stream_(nullptr) {
    }

    /**
     * \brief Constructs a blockwise processor with given overlap and initializes it with the given input stream, immediately loading the first block.
     * 
     * \param stream the input stream
     * \param block_size the size of an input block
     * \param overlap the overlap size
     */
    OverlappingBlocks(InputStream& stream, size_t const block_size, size_t const overlap) : OverlappingBlocks(block_size, overlap) {
        init(stream);
    }

    OverlappingBlocks(OverlappingBlocks&&) = default;
    OverlappingBlocks& operator=(OverlappingBlocks&&) = default;

    OverlappingBlocks(OverlappingBlocks const&) = delete;
    OverlappingBlocks& operator=(OverlappingBlocks const&) = delete;

    /**
     * \brief Initialize the blockwise processor with the given input stream, immmediately loading the first block.
     * 
     * Any potential rewinding of the stream has to be done beforehand.
     * For the first block, the overlap region is initialized with zeros.
     * 
     * \param stream the input stream
     */
    void init(InputStream& stream) {
        stream_ = &stream;

        // initialize the initial overlap region
        for(size_t i = 0; i < overlap_; i++) {
            buffer_[i] = 0;
        }

        // read the first block
        cur_offs_ = 0;
        read_next();
    }

    /**
     * \brief Advances the blockwise processor by loading the next block, memorizing the overlapping region of the current block.
     * 
     * \return true if the next block was loaded
     * \return false if the end of the input stream was reached
     */
    bool advance() {
        if(last()) return false;

        // slide overlap, then read next
        for(ssize_t i = 0; i < ssize_t(overlap_); i++) {
            buffer_[i] = cur_begin_[ssize_t(cur_size_) - ssize_t(overlap_) + i];
        }

        cur_offs_ += cur_size_;
        read_next();
        return cur_size_ > 0;
    }

    /**
     * \brief Accesses the character at the given position within the current block.
     * 
     * \param i the block-local position to access -- may be negative to access a character in the overlap region.
     * \return Char 
     */
    inline Char operator[](ssize_t const i) const{ return cur_begin_[i]; }

    inline Char const* begin() const { return cur_begin_; }
    inline Char const* end() const { return cur_begin_ + cur_size_; }

    /**
     * \brief Reports the current block's size, excluding the overlap.
     * 
     * \return size_t the current block's size, excluding the overlap.
     */
    inline size_t size() const { return cur_size_; }

    /**
     * \brief Reports the global offset of the current block.
     * 
     * \return size_t the global offset of the current block
     */
    inline size_t offset() const { return cur_offs_; }

    /**
     * \brief Tests whether the current block is empty.
     * 
     * \return true if the current block is empty
     * \return false otherwise
     */
    inline bool empty() const { return cur_size_ == 0; }

    /**
     * \brief Tests whether the current block is the first block.
     * 
     * \return true if the current block is the first block
     * \return false otherwise
     */
    inline bool first() const { return cur_offs_ == 0; }

    /**
     * \brief Tests whether the current block is the last block.
     * 
     * \return true if the current block is the last block
     * \return false otherwise
     */
    inline bool last() const { return probe_ == EOF_TOKEN; }

    /**
     * \brief Reports the size of the overlap region.
     * 
     * \return size_t the size of the overlap region
     */
    inline size_t overlap() const { return overlap_; }
};

}

#endif
