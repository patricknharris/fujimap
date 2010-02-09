/*
 * fujimap.hpp
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


#ifndef FUJIMAP_HPP__
#define FUJIMAP_HPP__

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include <fstream>
#include "fujimap_block.hpp"
#include "key_edge.hpp"

namespace fujimap_tool{

/**
  * Succinct Associative Array
  * Support basic key/value store operations (set/get)
  */

class Fujimap{
public:
  Fujimap(); ///< Default Constructor
  ~Fujimap(); ///< Default Destructor

  void initSeed(const uint64_t seed_); ///< Set a seed for hash function
  void initFP(const uint64_t fpWidth_); ///< Set a false postive rate (prob. of false positive is  2^{-fpWidth_})
  void initTmpN(const uint64_t tmpN); ///< Set a size of tempolary map

  void setString(const std::string& key, const std::string& value); ///< Store a (key, value). and seachable immediately
  void setStringTemporary(const std::string& key, const std::string& value); ///< Store a (key, value) and NOT seachable immediately
  
  void setInteger(const std::string& key,          const uint64_t value); ///< Store a (key, value). and seachable immediately
  void setIntegerTemporary(const std::string& key, const uint64_t value); ///< Store a (key, value) and NOT seachable immediately

  int build(); ///< Build a succinct associative array for a temporary associative array

  std::string getString(const std::string& key) const; ///< Return a value if exist, and return NOTFOUND if not.
  uint64_t getInteger(const std::string& key) const; ///< Return a value if exist, and return NOTFOUND if not.

  int load(const char* index); ///< Save a map in index
  int save(const char* index); ///< Load a map from index

  std::string what() const; ///< Report the current status (after error occured)
  size_t getKeyNum() const; ///< Return the number of registered keys.
private:
  uint64_t getCode(const std::string& value); ///< Return corresponding code of a given value
  void saveString(const std::string& s, std::ofstream& ofs) const; ///< Util for save
  void loadString(std::string& s, std::ifstream& ifs) const; ///< Util for load

  std::ostringstream what_;

  std::map<std::string, uint64_t> val2code; ///< Map from value to code
  std::vector<std::string> code2val; ///< Map from code to value

  std::vector<KeyEdge> keyEdges; ///< A set of non-searchable key/values to be indexed 
  std::map<std::string, uint64_t> tmpEdges; ///< A set of searchable key/values to be indexed

  std::vector< std::vector<FujimapBlock> > fbs; ///< Succinct Associative Arrays

  uint64_t seed; ///< Seed for hash
  uint64_t fpWidth; ///< A false positive rate (prob. of false psoitive is 2^{-fpWidth})
  uint64_t tmpN; ///< A size of tempolary map
};

}

#endif // FUJIMAP_HPP__
