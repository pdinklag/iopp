/**
 * file_input_stream.hpp
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

#ifndef _IOPP_FILE_INPUT_STREAM_HPP
#define _IOPP_FILE_INPUT_STREAM_HPP

#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

#include "stream_input_iterator.hpp"
#include "os/posix.hpp"

#ifdef IOPP_POSIX
#include <unistd.h>
#include <fcntl.h>
#else
#include <fstream>
#endif

namespace iopp {

/**
 * \brief \ref iopp::STLInputStreamLike "Standard input stream like" file input stream with a read buffer
 *
 * The default implementation is backed by a `std::ifstream` which is always opened in binary mode.
 * On POSIX systems, the faster POSIX file input API is used, also advising the kernel that the file is going to be read sequentially.
 */
class FileInputStream {
public:
    using pos_type = size_t;
    using off_type = ssize_t;
    using char_type = char;
    using int_type = int;

    static_assert(sizeof(char_type) == 1);

private:
    using uchar_type = unsigned char;

    size_t begin_;
    size_t end_;
    size_t bufsize_;

    std::unique_ptr<uchar_type[]> buffer_;
    size_t foffs_; // offset in file

#ifdef IOPP_POSIX
    int fd_;
#else
    std::ifstream fstream_;
#endif

    inline void invalidate_buffer() {
        setg(nullptr, nullptr, nullptr);
        gcount_ = 0;
        eof_ = false;
    }
    
    inline size_t fpos() const {
        return foffs_ + (gptr() - eback());
    }

    inline size_t bufcount() const {
        return egptr() - eback();
    }

    inline size_t view_size() const {
        return end_ - begin_;
    }

    // std::streambuf like internal interface
    // FileInputStream used to implement std::istream and std::streambuf and can easily be reverted to do so again
    bool eof_;
    uchar_type* eback_;
    uchar_type* gptr_;
    uchar_type* egptr_;
    size_t gcount_;

    inline uchar_type* eback() const { return eback_; }
    inline uchar_type* gptr() const { return gptr_; }
    inline uchar_type* egptr() const { return egptr_; }

    inline void setg(uchar_type* gbeg, uchar_type* gcurr, uchar_type* gend) {
        eback_ = gbeg;
        gptr_ = gcurr;
        egptr_ = gend;
    }

    inline size_t read_immediate(char_type* buffer, size_t const readnum) {
        ssize_t n;
        #ifdef IOPP_POSIX
        n = ::read(fd_, buffer, readnum);
        #else
        fstream_.read(buffer, readnum);
        n = fstream_.gcount();
        #endif
        return (n >= 0) ? n : 0;
    }

    inline int underflow() {
        foffs_ += bufcount();
        
        if(foffs_ < view_size()) {
            size_t const readnum = std::min(bufsize_, view_size() - foffs_);
            size_t const num_read = read_immediate((char_type*)buffer_.get(), readnum);

            if(num_read) {
                setg(buffer_.get(), buffer_.get(), buffer_.get() + num_read);
                return *(buffer_.get());
            }
        }

        return std::char_traits<char_type>::eof();
    }

    inline pos_type seekoff(off_type off, std::ios_base::seekdir dir) {
        // determine target position
        size_t new_foffs;
        switch(dir) {
            case std::ios::beg:
                new_foffs = off;
                break;
            
            case std::ios::cur:
                new_foffs = fpos() + off;
                break;
            
            case std::ios::end:
                new_foffs = view_size() + off;
                break;
        }
        
        if(new_foffs != foffs_) {
            foffs_ = new_foffs;

            #ifdef IOPP_POSIX
            lseek64(fd_, begin_ + foffs_, SEEK_SET);
            #else
            fstream_.seekg(begin_ + foffs_, std::ios::beg);
            #endif

            invalidate_buffer();
        }
        return fpos();
    }

    inline pos_type seekpos(pos_type pos) { return seekoff(pos, std::ios::beg); }

public:
    inline FileInputStream() : bufsize_(0), begin_(0), end_(0), foffs_(0), eof_(true), gcount_(0) {
        invalidate_buffer();

        #ifdef IOPP_POSIX
        fd_ = -1;
        #endif
    }

    /**
     * \brief Constructs a buffered file input stream for the specified file
     * 
     * Note that the construction alone will poll the file's size from the filesystem.
     * 
     * \param path the path to the file to read
     * \param begin the position of the first byte to read; this will be considered the beginning of the file, even if the file has prior data
     * \param end the position of the last byte to read; the stream will report EOF once this position is reached, even if the file is larger
     * \param bufsize the size of the read buffer
     */
    inline FileInputStream(std::filesystem::path const& path, size_t const begin = 0, size_t const end = SIZE_MAX, size_t const bufsize = 16384)
        : bufsize_(bufsize), begin_(begin), end_(end), foffs_(0), eof_(false), gcount_(0)
    {
        invalidate_buffer();

        if(!std::filesystem::exists(path)) {
            throw std::logic_error("file does not exist: " + path.string());
        }

        end_ = std::min(end_, std::filesystem::file_size(path));
        begin_ = std::min(begin_, end_);

        buffer_ = std::make_unique<uchar_type[]>(bufsize_);

        #ifdef IOPP_POSIX
        fd_ = open(path.c_str(), O_RDONLY);
        posix_fadvise(fd_, 0, 0, POSIX_FADV_SEQUENTIAL); // we expect sequential data access since this is a stream
        lseek64(fd_, begin_, SEEK_SET);
        #else
        fstream_ = std::ifstream(path, std::ios::in | std::ios::binary);
        fstream_.seekg(begin_, std::ios::beg);
        #endif
    }

    inline ~FileInputStream() {
        #ifdef IOPP_POSIX
        if(fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        #else
        // nothing to do, istream will be destroyed properly
        #endif
    }

    FileInputStream(FileInputStream const&) = delete;
    FileInputStream& operator=(FileInputStream const&) = delete;

    inline FileInputStream(FileInputStream&& other) : FileInputStream() {
        *this = std::move(other);
    }

    inline FileInputStream& operator=(FileInputStream&& other) {
        end_ = other.end_;
        bufsize_ = other.bufsize_;
        buffer_ = std::move(other.buffer_);
        foffs_ = other.foffs_;
        eof_ = other.eof_;
        setg(other.eback(), other.gptr(), other.egptr());

        #ifdef IOPP_POSIX
        fd_ = other.fd_;
        other.fd_ = -1;
        #else
        fstream_ = std::move(other.fstream_);
        #endif

        return *this;
    }

    /**
     * \brief Reads a single character
     * 
     * \return the read character, or \c std::char_traits<char>::eof() in case EOF has been reached
     */
    inline int_type get() {
        if(gptr_ < egptr_) {
            gcount_ = 1;
            return *gptr_++;
        } else {
            const auto x = underflow();
            if(x != std::char_traits<char_type>::eof()) {
                // all good
                eof_ = false;
                gcount_ = 1;
                return *gptr_++;
            } else {
                // EOF
                eof_ = true;
                gcount_ = 0;
                return x;
            }
        }
    }

    /**
     * \brief Reads multiple characters
     * 
     * The number of characters successfully read can be retrieved via \ref gcount .
     * 
     * \param outp the output buffer
     * \param num the number of characters to read
     * \return a reference to this stream
     */
    inline FileInputStream& read(char_type* outp, const size_t num) {    
        size_t const num_good = egptr_ - gptr_;
        if(num_good) {
            std::memcpy(outp, gptr_, std::min(num, num_good));
        }

        if(num <= num_good) {
            gptr_ += num;
            gcount_ = num;
        } else {
            // read remaining bytes directly
            size_t const remaining = num - num_good;
            gcount_ = num_good + read_immediate(outp + num_good, remaining);

            // account for that
            foffs_ += remaining;
            eof_ = gcount_ < num;
            setg(nullptr, nullptr, nullptr);
        }
        return *this;
    }

    /**
     * \brief Tests whether the stream is \em good
     * 
     * This is the case unless EOF has been reached after the last reading operation
     * 
     * \return true if there is still data available on the stream
     * \return false if EOF has been reached
     */
    inline bool good() const { return !eof_; }

    /**
     * \brief Equivalent to calling \ref good . 
     */
    explicit inline operator bool() const { return good(); }

    /**
     * \brief Reports the number of successfully read characters during the last \ref get or \ref read operation
     * 
     * \return size_t the number of read characters
     */
    inline size_t gcount() const { return gcount_; }
    
    /**
     * \brief Reports the next reading position in the stream
     * 
     * \return pos_type the next reading position in the stream
     */
    inline pos_type tellg() const {
        return fpos();
    }

    /**
     * \brief Seeks a stream position
     * 
     * \param off the position offset to seek
     * \param dir determines that the offset is applied to the beginning, current position ( \ref tellg ) or the end of the stream, respectively
     * \return FileInputStream& a reference to this stream
     */
    inline FileInputStream& seekg(off_type off, std::ios_base::seekdir dir) {
        seekoff(off, dir);
        return *this;
    }

    /**
     * \brief Returns a \ref StreamInputIterator over the file starting at the current stream position
     * 
     * \return an input iterator starting at the current stream position
     */
    inline auto begin() { return StreamInputIterator<FileInputStream>(*this); }

    /**
     * \brief Returns a \ref StreamInputIterator marking the end of the file
     * 
     * \return an input iterator marking the end of the file
     */
    inline auto end() { return StreamInputIterator<FileInputStream>::end(*this); }
};

}

#endif
