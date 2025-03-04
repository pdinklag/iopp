/**
 * load_file.hpp
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

#ifndef _IOPP_LOAD_FILE_HPP
#define _IOPP_LOAD_FILE_HPP

#include <algorithm>
#include <string>

#include "file_input_stream.hpp"
#include "stream_input_iterator.hpp"

namespace iopp {

/**
 * \brief Loads the specified file into a string
 * 
 * \param path the file path
 * \param prefix the maximum prefix to load from the file (defaults to whole file)
 * \return the file's contents
 */
inline std::string load_file_str(std::filesystem::path const& path, size_t const prefix = SIZE_MAX) {
    std::string s;
    s.reserve(std::min(std::filesystem::file_size(path), prefix));
    {
        FileInputStream fin(path);

        auto it = fin.begin();
        auto const end = fin.end();

        while(s.length() < prefix && it != end) {
            s.push_back(*it++);
        }
    }
    s.shrink_to_fit();
    return s;
}

}

#endif
