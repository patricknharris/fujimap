#ifndef KEYEDGE_HPP__
#define KEYEDGE_HPP__

#include "common.hpp"

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
  KeyEdge(const std::string& str, const uint32_t code,
	  const uint32_t seed);

  uint32_t get(uint32_t i, uint32_t bn) const{
    return (v[i] % bn) + bn * i;
  }

  uint32_t getBlock(){
    uint32_t x = 0;
    for (uint32_t i = 0; i < R; ++i){
      x ^= v[i];
    }
    return x % KEYBLOCK;
  }

  int operator < (const KeyEdge& k) const {
    for (int i = 0; i < R-1; ++i){
      if (v[i] != k.v[i]) return v[i] < k.v[i];
    }
    return v[R-1] < k.v[R-1];
  }

  void save(std::ofstream& ofs);
  void load(std::ifstream& ifs);

  uint32_t v[R];
  uint32_t code;

};

}

#endif // KEYEDGE_HPP__
