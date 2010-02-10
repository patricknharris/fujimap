#include "key_edge.hpp"

using namespace std;

namespace fujimap_tool{

  /*
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
*/

#define bob_mix(a, b, c) \
    a -= b; a -= c; a ^= (c>>43); \
    b -= c; b -= a; b ^= (a<<9); \
    c -= a; c -= b; c ^= (b>>8); \
    a -= b; a -= c; a ^= (c>>38); \
    b -= c; b -= a; b ^= (a<<23); \
    c -= a; c -= b; c ^= (b>>5); \
    a -= b; a -= c; a ^= (c>>35); \
    b -= c; b -= a; b ^= (a<<49); \
    c -= a; c -= b; c ^= (b>>11); \
    a -= b; a -= c; a ^= (c>>12); \
    b -= c; b -= a; b ^= (a<<18); \
    c -= a; c -= b; c ^= (b>>22); 



uint64_t get64bit(const uint8_t* v) {
  return (uint64_t)v[0] | ((uint64_t)v[1] << 8) | ((uint64_t)v[2] << 16) | ((uint64_t)v[3] << 24) | 
    ((uint64_t)v[4] << 32) | ((uint64_t)v[5] << 40) | ((uint64_t)v[6] << 48) | ((uint64_t)v[7] << 56);
}

void hash(const string& str, const uint64_t seed,  
	  uint64_t& a, uint64_t& b, uint64_t& c){
  const uint8_t* p = reinterpret_cast<const uint8_t*>(str.c_str());
  size_t len = str.size();

  a = 0x9e3779b97f4a7c13LL;
  b = seed;
  c = seed;
  while (len >= 24){
    a += get64bit(p + 0);
    b += get64bit(p + 8);
    c += get64bit(p + 16);
    bob_mix(a, b, c);
    p += 24;
    len -= 24;
  }

  c += static_cast<uint64_t>(str.size());
  switch(len)              /* all the case statements fall through */
    {
    case 23: c+=((uint64_t)p[22]<<56);
    case 22: c+=((uint64_t)p[21]<<48);
    case 21: c+=((uint64_t)p[20]<<40);
    case 20: c+=((uint64_t)p[19]<<32);
    case 19: c+=((uint64_t)p[18]<<24);
    case 18: c+=((uint64_t)p[17]<<16);
    case 17: c+=((uint64_t)p[16]<<8);
      /* the first byte of c is reserved for the length */
    case 16: b+=((uint64_t)p[15]<<56);
    case 15: b+=((uint64_t)p[14]<<48);
    case 14: b+=((uint64_t)p[13]<<40);
    case 13: b+=((uint64_t)p[12]<<32);
    case 12: b+=((uint64_t)p[11]<<24);
    case 11: b+=((uint64_t)p[10]<<16);
    case 10: b+=((uint64_t)p[ 9]<<8);
    case  9: b+=((uint64_t)p[ 8]);
    case  8: a+=((uint64_t)p[ 7]<<56);
    case  7: a+=((uint64_t)p[ 6]<<48);
    case  6: a+=((uint64_t)p[ 5]<<40);
    case  5: a+=((uint64_t)p[ 4]<<32);
    case  4: a+=((uint64_t)p[ 3]<<24);
    case  3: a+=((uint64_t)p[ 2]<<16);
    case  2: a+=((uint64_t)p[ 1]<<8);
    case  1: a+=((uint64_t)p[ 0]);
    default : ;
    }

  bob_mix(a, b, c);
}


KeyEdge::KeyEdge(const string& str, const uint64_t code, 
		 const uint64_t seed) : v(R), code(code){
  hash(str, seed, v[0], v[1], v[2]);
}

  KeyEdge::KeyEdge() :  v(R), code(0){
}

void KeyEdge::save(ofstream& ofs){
  ofs.write((const char*)(&code), sizeof(code));
  ofs.write((const char*)(&v[0]), sizeof(v[0]) * R);
}

void KeyEdge::load(ifstream& ifs){
  ifs.read((char*)(&code), sizeof(code));
  ifs.read((char*)(&v[0]), sizeof(v[0]) * R);

}

}
