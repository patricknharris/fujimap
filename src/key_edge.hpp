#ifndef KEYEDGE_HPP__
#define KEYEDGE_HPP__

#include "fujimap_common.hpp"

#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

#include <iostream>

namespace fujimap_tool{

/*
 * Intermediated representation of key/value
 * used in HashMap. 
 */

uint64_t hash(const char* str, size_t len);
void hash(const char* str, const size_t len, const uint64_t seed,  
		 uint64_t& a, uint64_t& b, uint64_t& c);

struct KeyEdge{
  KeyEdge(); 
  KeyEdge(const char* str, const size_t len, const uint64_t code,
	  const uint64_t seed); 

  uint64_t get(uint64_t i, uint64_t bn) const{
    return (v[i] % bn) + bn * i;
  }

  uint64_t getRaw(uint64_t i, uint64_t bn) const{
    return v[i] % bn;
  }

  bool operator < (const KeyEdge& k) const {
    for (uint64_t i = 0; i < R; ++i){
      if (v[i] != k.v[i]) return v[i] < k.v[i];
    }
    return false;
  }

  void save(std::ofstream& ofs);
  void load(std::ifstream& ifs);

  uint64_t v[3];
  uint64_t code;
};

}

#endif // KEYEDGE_HPP__
