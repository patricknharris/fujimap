/*
 * common.hpp
 * Copyright (c) 2010 Daisuke Okanohara All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FUJIMAP_COMMON_HPP__
#define FUJIMAP_COMMON_HPP__

#include <stdint.h>

namespace fujimap_tool{
  static const uint32_t R = 3;        ///< Hash Num in fujimap_block
  static const uint32_t FPWIDTH = 10; ///< Default width of false positive bit
  static const uint32_t TMPN = 1000; ///< Default size of temporary associative array
  static const uint32_t KEYBLOCK = 10000000;  ///< # of minimum perfect hash function in fujimap_block
  static const uint32_t NOTFOUND = 0xFFFFFFFF; ///< Indicate that key is not found
}

#endif // COMMON_HPP__
