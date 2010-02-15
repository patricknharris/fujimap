/*
 * fujimap_block.cpp
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

#include <fstream>
#include <queue>
#include <cassert>
#include <iostream>
#include <algorithm>
#include "fujimap_block.hpp"

using namespace std;

namespace fujimap_tool{

const double   FujimapBlock::C_R = 1.3;


FujimapBlock::FujimapBlock() : 
  keyNum(0), codeNum(0), codeWidth(0), fpWidth(FPWIDTH), seed(0x12345678), 
  bn(0){
}

FujimapBlock::~FujimapBlock(){
}

uint64_t FujimapBlock::getVal(const string& key) const{
  return getVal(KeyEdge(key, 0, seed));
}

uint64_t FujimapBlock::getVal(const KeyEdge& ke) const{
  if (B.bvSize() == 0){
    return NOTFOUND;
  }
  uint64_t codeLen = fpWidth + codeWidth;
  uint64_t bits = 0;
  for (uint64_t i = 0; i < R; ++i){
    bits ^= B.getBits(ke.get(i, bn) * codeLen, codeLen);
  }

  if ((bits & ((1LLU << fpWidth)-1)) != ke.getRaw(1, 1LLU << fpWidth)){
    return NOTFOUND;
  }

  uint64_t code = bits >> fpWidth;
  if (code > codeNum){
    return NOTFOUND;
  }

  return code;
}

uint64_t FujimapBlock::log2(uint64_t x){
  if (x == 0) return 0;
  --x;
  
  uint64_t ret = 1;
  while (x >> ret){
    ++ret;
  }
  return ret;
}



int FujimapBlock::build_(vector<KeyEdge>& keyEdges){
  // set keyEdge
  const uint64_t codeLen = codeWidth + fpWidth;
  assert(codeLen <= 63);
  vector<uint64_t> edges(keyNum * R);
  vector<uint8_t>  degs(bn * R);
  vector<uint64_t> offsets(bn * R + 1);

  for (size_t i = 0; i < keyEdges.size(); ++i){
    const KeyEdge& ke(keyEdges[i]);
    for (uint64_t j = 0; j < R; ++j){
      uint64_t t = ke.get(j, bn);
      if (degs[t] == 0xFF) {
	return -1;
      }
      degs[t]++;
    }
  }

  uint64_t sum = 0;
  //BitVec offset(keyNum * codeLen * R + 1 + totalBN);
  for (size_t i = 0; i < degs.size(); ++i){
    offsets[i] = sum;
    sum += degs[i];
    degs[i] = 0;
  }
  //offset.setBit(sum + totalBN);
  //offset.buildSelect();
  offsets.back() = sum;
  assert(sum == keyNum * R);

  for (size_t i = 0; i < keyEdges.size(); ++i){
    const KeyEdge& ke(keyEdges[i]);
    for (uint64_t j = 0; j < R; ++j){
      uint64_t t = ke.get(j, bn);
      edges[offsets[t] + degs[t]++] = i;
    }
  }

  BitVec visitedEdges(keyNum);
  std::queue<uint64_t> q;
  for (size_t i = 0; i < degs.size(); ++i){
    if (degs[i] == 1) {
      q.push(edges[offsets[i]]);
      assert(offsets[i+1] - offsets[i] == 1);
    }
  }

  vector<pair<uint64_t, uint8_t> > extractedEdges;
  uint64_t deletedNum = 0;

  while (!q.empty()){
    uint64_t v = q.front();
    q.pop();
    if (visitedEdges.getBit(v)) continue;
    visitedEdges.setBit(v);
    ++deletedNum;

    const KeyEdge& ke(keyEdges[v]);
    int choosed = -1;
    for (uint64_t i = 0; i < R; ++i){
      const uint64_t t = ke.get(i, bn);
      --degs[t];

      if (degs[t] == 0){
	choosed = i;
	continue;
      } else if (degs[t] >= 2) {
	continue;
      }
      // degs[t] == 1
      uint64_t end = offsets[t+1];
      for (uint64_t j = offsets[t]; j < end; ++j){
	if (!visitedEdges.getBit(edges[j])){
	  q.push(edges[j]);
	  break;
	}
      }
    }
    assert(choosed != -1);
    extractedEdges.push_back(make_pair(v, choosed));
  }

  if (deletedNum != keyNum){
    cerr << "assignment error deletedNum:" << deletedNum <<
      "keyNum:" << keyNum << " seed:" << seed << endl;
    return -1;
  }
  assert(q.empty());

  B.resize(bn * codeLen * R);


  BitVec visitedVerticies(bn * R);
  reverse(extractedEdges.begin(), extractedEdges.end());
  for (vector<pair<uint64_t, uint8_t> >::const_iterator it = 
	 extractedEdges.begin(); it != extractedEdges.end(); ++it){
    const uint64_t v       = it->first;
    const KeyEdge& ke(keyEdges[v]);

    uint64_t mask = ke.getRaw(1, 1LLU << fpWidth);
    uint64_t bits = (ke.code << fpWidth) + mask; // ke.code 1111

    //BitVec::printBit(bits, codeLen);

    for (uint64_t i = 0; i < R; ++i){
      const uint64_t t = ke.get(i, bn);
      if (!(visitedVerticies.getBit(t))){
	continue;
      }
      bits ^= B.getBits(t * codeLen, codeLen);
    }

    const uint64_t setPos = ke.get(it->second, bn);
    B.setBits(setPos * codeLen, codeLen, bits);
    visitedVerticies.setBit(setPos);
  }

  return 0;
}


int FujimapBlock::build(vector<KeyEdge>& keyEdges,
			const uint64_t seed_, const uint64_t fpWidth_){
  seed     = seed_;
  keyNum   = static_cast<uint64_t>(keyEdges.size());
  fpWidth  = fpWidth_;

  codeNum   = 0;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    if (keyEdges[i].code >= codeNum){
      codeNum = keyEdges[i].code + 1;
    }
  }
  codeWidth = log2(codeNum);
  bn        = (uint64_t)(keyNum * C_R / (double)R + 100);

  /*
  cerr << " keyNum:" << keyNum << endl
       << "codeNum:" << codeNum << endl
       << "codeWid:" << codeWidth << endl
       << "     bn:" << bn << endl
       << "fpWidth:" << fpWidth << endl;
  */

  bool succeeded = false;
  for (int iter = 0; iter < 20; ++iter){
    if (build_(keyEdges) == 0){
      succeeded = true;
      break;
    }
    seed++;
  }
  if (succeeded) return 0;
  else return -1;
}

size_t FujimapBlock::getKeyNum() const {
  return keyNum;
}

void FujimapBlock::save(ofstream& ofs){
  ofs.write((const char*)(&keyNum), sizeof(keyNum));
  ofs.write((const char*)(&codeNum), sizeof(codeNum));
  ofs.write((const char*)(&fpWidth), sizeof(fpWidth));
  ofs.write((const char*)(&seed), sizeof(seed));
  B.write(ofs);
}

void FujimapBlock::load(ifstream& ifs){
  ifs.read((char*)(&keyNum), sizeof(keyNum));
  ifs.read((char*)(&codeNum), sizeof(codeNum));
  ifs.read((char*)(&fpWidth), sizeof(fpWidth));
  ifs.read((char*)(&seed), sizeof(seed));
  B.read(ifs);

  codeWidth = log2(codeNum);
  bn        = (uint64_t)(keyNum * C_R / (double)R + 100);
}

}
