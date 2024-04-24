/**
 * file_output_stream.hpp
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

#ifndef _IOPP_FILE_OUTPUT_STREAM_HPP
#define _IOPP_FILE_OUTPUT_STREAM_HPP

#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

#include "os/posix.hpp"

#ifdef IOPP_POSIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <fstream>
#endif

namespace iopp {

/**
 * \brief \ref iopp::STLOutputStreamLike "Standard output stream like" file output stream with a write buffer
 * 
 * The default implementation is backed by a `std::ofstream` which is always opened in binary mode.
 * On POSIX systems, the faster POSIX file input API is used, also advising the kernel that the file is going to be written sequentially.
 * 
 * This output stream does \em not support seek operations.
 */
class FileOutputStream {
public:
    using pos_type = size_t;
    using off_type = ssize_t;
    using char_type = char;
    using int_type = int;

private:
    using uchar_type = unsigned char;
    size_t bufsize_;

    std::unique_ptr<uchar_type[]> buffer_;
    size_t foffs_; // offset in file

#ifdef IOPP_POSIX
    int fd_;
#else
    std::ofstream fstream_;
#endif
    
    inline size_t fpos() const {
        return foffs_ + (pptr() - pbase());
    }

    inline size_t bufcount() const {
        return pptr() - pbase();
    }

    // std::streambuf like internal interface
    // FileOutputStream used to implement std::ostream and std::streambuf and can easily be reverted to do so again
    uchar_type* pbase_;
    uchar_type* pptr_;
    uchar_type* epptr_;

    inline uchar_type* pbase() const { return pbase_; }
    inline uchar_type* pptr() const { return pptr_; }
    inline uchar_type* epptr() const { return epptr_; }

    inline void setp(uchar_type* pbeg, uchar_type* pend) {
        pbase_ = pbeg;
        pptr_ = pbeg;
        epptr_ = pend;
    }

    inline int sync() {
        // write current buffer contents to file
        size_t const wnum = bufcount();

        #ifdef IOPP_POSIX
        size_t num_written = 0;
        if(fd_ >= 0) {
            while(num_written < wnum) {
                ssize_t const w = ::write(fd_, buffer_.get(), wnum - num_written);
                if(w > 0) [[likely]] {
                    num_written += w;
                } else {
                    break; // we may want to throw here?
                }
            }
        }
        #else
            fstream_.write((char*)buffer_.get(), wnum);
            size_t const num_written = wnum; // fstream would have thrown if any error occurred
        #endif

        // advance file position
        foffs_ += num_written;

        // reset write buffer
        setp(buffer_.get(), buffer_.get() + bufsize_);

        // return success
        return 0;
    }
 
public:
    inline FileOutputStream() : bufsize_(0), foffs_(0) {
        setp(nullptr, nullptr);

        #ifdef IOPP_POSIX
        fd_ = -1;
        #endif
    }

    /**
     * \brief Constructs a buffered file output stream for the specified file
     * 
     * If the file already exists, it will be overwritten.
     * 
     * \param path the path to the file to write
     * \param bufsize the size of the write buffer
     */
    inline FileOutputStream(std::filesystem::path const& path, size_t const bufsize = 16384) : bufsize_(bufsize), foffs_(0) {
        buffer_ = std::make_unique<uchar_type[]>(bufsize_);
        setp(buffer_.get(), buffer_.get() + bufsize_);

        #ifdef IOPP_POSIX
        const auto mask = umask(0);
        umask(mask); // needed to restore as per POSIX documentation
        fd_ = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666 & ~mask);
        posix_fadvise(fd_, 0, 0, POSIX_FADV_SEQUENTIAL); // we expect sequential data access since this is a stream
        #else
        fstream_ = std::ofstream(path, std::ios::out | std::ios::binary);
        #endif
    }

    inline ~FileOutputStream() {
        sync();

        #ifdef IOPP_POSIX
        if(fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        #else
        // nothing to do, ostream will be destroyed properly
        #endif
    }

    FileOutputStream(FileOutputStream const&) = delete;
    FileOutputStream& operator=(FileOutputStream const&) = delete;

    inline FileOutputStream(FileOutputStream&& other) : FileOutputStream() {
        *this = std::move(other);
    }

    inline FileOutputStream& operator=(FileOutputStream&& other) {
        bufsize_ = other.bufsize_;
        buffer_ = std::move(other.buffer_);
        foffs_ = other.foffs_;
        
        pbase_ = other.pbase_;
        pptr_ = other.pptr_;
        epptr_ = other.epptr_;

        #ifdef IOPP_POSIX
        fd_ = other.fd_;
        other.fd_ = -1;
        #else
        fstream_ = std::move(other.fstream_);
        #endif

        return *this;
    }

    /**
     * \brief Writes a single character
     * 
     * \return a reference to this stream
     */
    inline FileOutputStream& put(const char_type c) {
        if(pptr_ >= epptr_) {
            // flush
            sync();
        }

        *pptr_++ = c;
        return *this;
    }

    /**
     * \brief Writes multiple characters
     * 
     * \return a reference to this stream
     */
    inline FileOutputStream& write(char_type const* inp, const size_t num) {
        size_t written = 0;
        while(written < num) {
            while(written < num && pptr_ < epptr_) {
                *pptr_++ = *inp++;
                ++written;
            }

            if(written < num) {
                sync(); // flush when necessary
            }
        }
        return *this;
    }

    /**
     * \brief Forces the write buffer to be flushed to the output file
     * 
     * \return a reference to this stream
     */
    inline FileOutputStream& flush() {
        sync();
        return *this;
    }

    /**
     * \brief Reports the next write position in the file
     * 
     * \return the next write position in the file
     */
    inline pos_type tellp() const {
        return fpos();
    }
};

}

#endif
