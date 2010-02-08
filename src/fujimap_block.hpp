/*
 * fujimap_block.hpp
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

#ifndef FUJIMAP_BLOCK_HPP__
#define FUJIMAP_BLOCK_HPP__

#include <vector>
#include <string>
#include <fstream>
#include "key_edge.hpp"
#include "bitvec.hpp"
#include "fujimap_common.hpp"

namespace fujimap_tool{

/*
 * Minimum Perfect Associative Array
 * used in Fujimap
 */
class FujimapBlock{
  static const double C_R; ///< Redundancy for bit array (>1.3)

public:
  FujimapBlock(); ///< Default Constructor
  ~FujimapBlock(); ///< Default Destructor

  int build(std::vector<KeyEdge>& keyEdges,
	    const uint32_t seed, const uint32_t fpWidth); ///< build a associative map
  uint32_t getVal(const std::string& key) const; ///< return a value corresponding to the given key
  uint32_t getVal(const KeyEdge& ke) const; ///< return a value corresponding to the given KeyEdge
  void save(std::ofstream& ofs); ///< save the status in ofs
  void load(std::ifstream& ifs); ///< load the status from ifs

  size_t getKeyNum() const; ///<return the number of registered keys

private:
  int build_(std::vector<KeyEdge>& keyEdges);

  uint32_t log2(uint32_t x);
  void test();

  BitVec B;

  uint32_t keyNum;
  uint32_t codeNum;
  uint32_t codeWidth;
  uint32_t fpWidth;
  uint32_t seed;
  uint32_t bn;


};

}

#endif // FUJIMAP_BLOCK_HPP__
