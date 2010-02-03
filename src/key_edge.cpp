#include "key_edge.hpp"

using namespace std;

namespace fujimap_tool{

#define bob_mix(a, b, c) \
  a -= b; a -= c; a ^= (c >> 13);      \
  b -= c; b -= a; b ^= (a << 8);        \
  c -= a; c -= b; c ^= (b >> 13);       \
  a -= b; a -= c; a ^= (c >> 12);       \
  b -= c; b -= a; b ^= (a << 16);       \
  c -= a; c -= b; c ^= (b >> 5);        \
  a -= b; a -= c; a ^= (c >> 3);        \
  b -= c; b -= a; b ^= (a << 10);       \
  c -= a; c -= b; c ^= (b >> 15);


uint32_t get32bit(const uint8_t* v) {
  return v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
}

void hash(const string& str, const uint32_t seed,  
	  uint32_t& a, uint32_t& b, uint32_t& c){
  const uint8_t* p = reinterpret_cast<const uint8_t*>(str.c_str());
  size_t len = str.size();

  a = 0x9e3779b9;
  b = 0x9e3779b9;
  c = seed;
  while (len >= 12){
    a += get32bit(p + 0);
    b += get32bit(p + 4);
    c += get32bit(p + 8);
    bob_mix(a, b, c);
    p += 12;
    len -= 12;
  }

  c += static_cast<uint32_t>(str.size());
  switch (len){
  case 11: c += p[10] << 24;
  case 10: c += p[9]  << 16;
  case  9: c += p[8]  << 8;
  case  8: b += p[7]  << 24;
  case  7: b += p[6]  << 16;
  case  6: b += p[5]  << 8;
  case  5: b += p[4];
  case  4: a += p[3]  << 24;
  case  3: a += p[2]  << 16;
  case  2: a += p[1]  << 8;
  case  1: a += p[0];
  default :  ;
  }

  bob_mix(a, b, c);
}


KeyEdge::KeyEdge(const string& str, const uint32_t code, 
		 const uint32_t seed) : code(code){
  hash(str, seed, v[0], v[1], v[2]);
}

KeyEdge::KeyEdge(){
}

void KeyEdge::save(ofstream& ofs){
  ofs.write((const char*)(&code), sizeof(code));
  for (uint32_t i = 0; i < R; ++i){
    ofs.write((const char*)(&v[i]), sizeof(v[i]));
  }
}

void KeyEdge::load(ifstream& ifs){
  ifs.read((char*)(&code), sizeof(code));
  for (uint32_t i = 0; i < R; ++i){
    ifs.read((char*)(&v[i]), sizeof(v[i]));
  }
}

}
