#include <iostream>
#include <vector>
#include <cstdlib>
#include "bitvec.hpp"

using namespace std;

int main(int argc, char* argv[]){
  
  const int n = 1000000;
  vector<uint32_t> v;
  vector<uint32_t> sv;

  BitVec bv(n);
  for (int i = 0; i < n; ++i){
    int b = rand() % 2;
    v.push_back(b);
    if (b){
      sv.push_back(i);
      bv.setBit(i);
    }
  }

  // check
  cerr << "getBit test" << endl;
  for (int i = 0; i < n; ++i){
    if (v[i] != bv.getBit(i)){
      cerr << "error i:" << i << " v[i]:" << v[i] << " bv.getBit(i):" << bv.getBit(i) << endl;
      return -1;
    }
  }
  cerr << "done" << endl;

  cerr << "getBits test" << endl;
  for (int i = 0; i < n; ++i){
    uint32_t code = 0;
    for (uint32_t j = 0; j < 10; ++j){
      code |= (bv.getBit(i + j)) << j;
    }
    uint32_t code2 = bv.getBits(i, 10);
    if (code != code2) {
      cerr << "error i:" << i << " code:" << code <<" getBits:" << code2 << endl;
      for (int j = 0; j < 10; ++j){
	cerr << ((code >> j) & 1U) <<  " " << ((code2 >> j) & 1U) << endl;
      }
      return -1;
    }
  }

  cerr << "done" << endl;
  

  return 0;
}
