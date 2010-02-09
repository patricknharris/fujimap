#ifndef KEYEDGE_HPP__
#define KEYEDGE_HPP__

#include "fujimap_common.hpp"

#include <stdint.h>
#include <string>
#include <fstream>

namespace fujimap_tool{

/*
 * Intermediated representation of key/value
 * used in HashMap. 
 */
struct KeyEdge{
  KeyEdge();
  KeyEdge(const std::string& str, const uint64_t code,
	  const uint64_t seed);

  uint64_t get(uint64_t i, uint64_t bn) const{
    return (v[i] % bn) + bn * i;
  }

  uint64_t getBlock(const uint64_t blockNum){
    uint64_t x = 0;
    for (uint32_t i = 0; i < R; ++i){
      x ^= v[i];
    }
    return x % blockNum;
  }

  int operator < (const KeyEdge& k) const {
    for (int i = 0; i < R-1; ++i){
      if (v[i] != k.v[i]) return v[i] < k.v[i];
    }
    return v[R-1] < k.v[R-1];
  }

  void save(std::ofstream& ofs);
  void load(std::ifstream& ifs);

  uint64_t v[R];
  uint64_t code;
};

}

#endif // KEYEDGE_HPP__
