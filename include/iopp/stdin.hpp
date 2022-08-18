/**
 * stdin.hpp
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

#ifndef _IOPP_STDIN_HPP
#define _IOPP_STDIN_HPP

#include "os/posix.hpp"
#include "os/windows.hpp"

#ifdef IOPP_POSIX
#include <stdio.h>
#include <unistd.h>
#elif defined(IOPP_WINDOWS)
#include <io.h>
#include <stdio.h>
#endif

namespace iopp {

/**
 * \brief Tests whether the standard input is receiving input from a pipe
 * 
 * This is tested using the `isatty` function provided by the OS.
 * 
 * \return true if the standard input is receiving input from a pipe
 * \return false otherwise
 */
inline bool stdin_is_pipe() {
    #ifdef IOPP_POSIX
    return !isatty(fileno(stdin));
    #elif defined(IOPP_WINDOWS)
    return !_isatty(_fileno(stdin));
    #endif
}

}

#endif
