/**
 * memory_mapped_file.hpp
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

#ifndef _IOPP_MEMORY_MAPPED_HPP
#define _IOPP_MEMORY_MAPPED_HPP

#include <filesystem>
#include <utility>
#include "os/posix.hpp"

#ifdef IOPP_POSIX
    #include <unistd.h>
    #ifdef _POSIX_MAPPED_FILES
        #include <fcntl.h>
        #include <sys/stat.h>
        #include <sys/mman.h>

        #define IOPP_POSIX_MMAP
    #endif
#endif

namespace iopp {

/**
 * \brief Represents a memory-mapped file.
 */
class MemoryMappedFile {
private:
    #ifdef IOPP_POSIX_MMAP
    int    fd_;
    #endif
    
    size_t size_;
    void*  data_;

public:
    /**
     * \brief Tests whether memory mapping is available on this system
     * 
     * This is true on Unix systems that define `_POSIX_MAPPED_FILES` in `unistd.h`.
     * 
     * \return true if memory mapping is supported
     * \return false otherwise
     */
    static constexpr bool available() {
        #ifdef IOPP_POSIX_MMAP
        return true;
        #else
        return false;
        #endif
    }

    inline MemoryMappedFile() : size_(0), data_(nullptr) {
        #ifdef IOPP_POSIX_MMAP
        fd_ = -1;
        #endif
    }

    inline ~MemoryMappedFile() {
        #ifdef IOPP_POSIX_MMAP
        if(data_) {
            munmap(data_, size_);
            data_ = nullptr;
        }
        
        if(fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }

        size_ = 0;
        #endif
    }

    /**
     * \brief Memory-maps the file at the specified path.
     * 
     * \param path the file to memory-map
     * \param begin the position of the first byte to map
     * \param end the position of the last byte to map
     */
    inline MemoryMappedFile(std::filesystem::path const& path, size_t const begin = 0, size_t const end = SIZE_MAX) {
        #ifdef IOPP_POSIX_MMAP
        fd_ = open(path.c_str(), O_RDONLY);
        if(fd_ >= 0) {
            struct stat st;
            if(fstat(fd_, &st) >= 0) {
                size_t const actual_end = std::min(end, (size_t)st.st_size);
                size_t const actual_begin = std::min(begin, actual_end);
                size_t const len = actual_end - actual_begin;

                void* data = mmap64(nullptr, len, PROT_READ, MAP_PRIVATE, fd_, actual_begin);
                if(data != MAP_FAILED) {
                    data_ = data;
                    size_ = len;
                }
            }
        }
        #endif
    }

    inline MemoryMappedFile(MemoryMappedFile&& other) : MemoryMappedFile() {
        *this = std::move(other);
    }

    inline MemoryMappedFile& operator=(MemoryMappedFile&& other) {
        size_ = other.size_;
        other.size_ = 0;
        
        data_ = other.data_;
        other.data_ = nullptr;
        
        #ifdef IOPP_POSIX_MMAP
        fd_ = other.fd_;
        other.fd_ = -1;
        #endif
        
        return *this;
    }

    MemoryMappedFile(MemoryMappedFile const&) = delete;
    MemoryMappedFile& operator=(MemoryMappedFile const&) = delete;

    /**
     * \brief Provides access to the file's mapped memory area
     * 
     * \return a pointer to the file's mapped memory area
     */
    inline void const* data() const {
        return data_;
    }

    /**
     * \brief Reports the size of the memory-mapped area
     * 
     * \return the size of the file's mapped memory area
     */
    inline const size_t size() const {
        return size_;
    }
};

}

#endif
