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
  KeyEdge ke(key, 0, seed);
  return getVal(ke);
}

uint64_t FujimapBlock::getVal(const KeyEdge& ke) const{
  if (B.bvSize() == 0){
    return NOTFOUND;
  }
  uint64_t code = 0;

  uint64_t checkCode = 0;
  for (uint64_t i = 0; i < fpWidth; ++i){
    uint64_t bit = 0;
    for (uint64_t j = 0; j < R; ++j){
      bit ^= B.getBit(ke.get(j, bn) + i);
    }
    checkCode |= (bit << i);
  }

  if (checkCode != (1U << fpWidth) - 1){
    return NOTFOUND;
  }

  for (uint64_t i = 0; i < codeWidth; ++i){
    uint64_t bit = 0;
    for (uint64_t j = 0; j < R; ++j){
      bit ^= B.getBit(ke.get(j, bn) + i + fpWidth);
    }
    code |= (bit << i);
  }

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
  B.resize(bn * R);
  uint64_t totalBN = B.bvSize() * 64;
  uint64_t codeLen = codeWidth + fpWidth;
  vector<uint8_t> deg(totalBN);
  vector<uint64_t> offset(totalBN+1);
  vector<uint64_t> edges(keyNum * codeLen * R);
  BitVec visitedVerticies(totalBN);

  cerr << "deg check" << endl;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    const KeyEdge& ke(keyEdges[i]);
    for (uint64_t j = 0; j < R; ++j){
      for (uint64_t k = 0; k < codeLen; ++k){
	uint64_t t = (ke.get(j, bn) + k) % totalBN;
	if (deg[t] == 0xFF) {
	  return -1;
	}
	deg[t]++;
      }
    }
  }

  uint64_t sum = 0;
  //BitVec offset(keyNum * codeLen * R + 1 + totalBN);
  for (uint64_t i = 0; i < totalBN; ++i){
    //offset.setBit(sum + i);
    offset[i] = sum;
    sum += deg[i];
    deg[i] = 0;
  }
  //offset.setBit(sum + totalBN);
  //offset.buildSelect();
  offset[totalBN] = sum;
  assert(sum == keyNum * codeLen * R);

  cerr << "edge set" << endl;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    const KeyEdge& ke(keyEdges[i]);
    for (uint64_t j = 0; j < R; ++j){
      for (uint64_t k = 0; k < codeLen; ++k){
	uint64_t t = (ke.get(j, bn) + k) % totalBN;
	assert(offset[t] + deg[t] < edges.size());
	edges[offset[t] + deg[t]] = i * codeLen + k;
	deg[t]++;
      }
    }
  }

  vector<uint8_t> visitedEdges(keyNum * codeLen);
  std::queue<uint64_t> q;
  for (uint64_t i = 0; i < totalBN; ++i){
    if (deg[i] == 1) {
      uint64_t t = edges[offset[i]];
      q.push(t);
      assert(offset[i+1] - offset[i] == 1);
    }
  }

  cerr << "extractEdge" << endl;
  vector<pair<uint64_t, uint8_t> > extractedEdges;
  uint64_t deletedNum = 0;

  size_t searchNum = 0;
  while (!q.empty()){
    uint64_t v       = q.front();
    q.pop();
    //if (visitedEdges.getBit(v)) continue;
    if (visitedEdges[v]) continue;
    uint64_t keyID   = v / codeLen;
    uint64_t codePos = v % codeLen;
    visitedEdges[v] = 1;
    ++deletedNum;

    const KeyEdge& ke(keyEdges[keyID]);
    int choosed = -1;
    for (uint64_t i = 0; i < R; ++i){
      const uint64_t t = (ke.get(i, bn) + codePos) % totalBN;
      --deg[t];
      if (deg[t] == 0){
	choosed = i;
      } else if (deg[t] == 1) {
	uint64_t end = offset[t+1];
	for (uint64_t j = offset[t]; j < end; ++j){
	  searchNum++;
	  if (!visitedEdges[edges[j]]){
	    q.push(edges[j]);
	    break;
	  }
	}
      }
    }
    assert(choosed != -1);
    extractedEdges.push_back(make_pair(v, choosed));
  }

  if (deletedNum != keyNum * codeLen){
    return -1;
  }
  assert(q.empty());

  cerr << searchNum << " " << (double)searchNum / deletedNum << endl;

  cerr << "setBit" << endl;
  reverse(extractedEdges.begin(), extractedEdges.end());
  for (vector<pair<uint64_t, uint8_t> >::const_iterator it = 
	 extractedEdges.begin(); it != extractedEdges.end(); ++it){
    const uint64_t v       = it->first;
    const uint64_t keyID   = v / codeLen;
    const uint64_t codePos = v % codeLen;
    const KeyEdge& ke(keyEdges[keyID]);

    uint64_t bit = (codePos < fpWidth) ? 1U :
      (ke.code >> (codePos - fpWidth)) & 1U;
    for (uint64_t i = 0; i < R; ++i){
      const uint64_t t = (ke.get(i, bn) + codePos) % totalBN;
      if (!(visitedVerticies.getBit(t))){
	continue;
      }
      bit ^= B.getBit(t);
    }

    const uint64_t setPos = (ke.get(it->second, bn) + codePos);
    if (bit){
      B.setBit(setPos);
    }
    visitedVerticies.setBit(setPos);
  }

  cerr << "end" << endl;
  return 0;
}


int FujimapBlock::build(vector<KeyEdge>& keyEdges,
			const uint64_t seed_, const uint64_t fpWidth_){
  seed     =  seed_;
  keyNum   = static_cast<uint64_t>(keyEdges.size());
  fpWidth  = fpWidth_;

  codeNum   = 0;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    if (keyEdges[i].code >= codeNum){
      codeNum = keyEdges[i].code + 1;
    }
  }
  codeWidth = log2(codeNum);
  bn        = (uint64_t)(keyNum * (codeWidth + fpWidth) * C_R / (double)R + 100);

  cerr << " keyNum:" << keyNum << endl
       << "codeNum:" << codeNum << endl
       << "codeWid:" << codeWidth << endl
       << "     bn:" << bn << endl
       << "fpWidth:" << fpWidth << endl;


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
  bn        = (uint64_t)(keyNum * (codeWidth + fpWidth) * C_R / (double)R + 100);
}

}
