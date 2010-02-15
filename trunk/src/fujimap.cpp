/*
 * fujimap.cpp
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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>
#include "fujimap.hpp"


using namespace std;

namespace fujimap_tool{

Fujimap::Fujimap() :  seed(0x123456), fpWidth(FPWIDTH), tmpN(TMPN), keyBlockN(KEYBLOCK) {
  keyEdges.resize(KEYBLOCK);
}
Fujimap::~Fujimap(){
}

void Fujimap::initSeed(const uint64_t seed_){
  seed = seed_;
}

void Fujimap::initFP(const uint64_t fpWidth_){
  fpWidth = fpWidth_;
}

void Fujimap::initTmpN(const uint64_t tmpN_){
  tmpN = tmpN_;
}

void Fujimap::initKeyBlockN(const uint64_t keyBlockN_){
  keyBlockN = keyBlockN_;
  keyEdges.resize(keyBlockN);
}

void Fujimap::setString(const std::string& key, const std::string& value){
  tmpEdges[key] = getCode(value);
  if (tmpEdges.size() == tmpN){
    build();
  }
}

void Fujimap::setStringTemporary(const std::string& key, const std::string& value){
  KeyEdge k(key, getCode(value), seed);
  keyEdges[k.getRaw(0, keyBlockN)].push_back(k);
}

void Fujimap::setInteger(const std::string& key, const uint64_t value){
  tmpEdges[key] = value;
  if (tmpEdges.size() == tmpN){
    build();
  } 
}

void Fujimap::setIntegerTemporary(const std::string& key, const uint64_t value){
  KeyEdge k(key, value, seed);
  keyEdges[k.getRaw(0, keyBlockN)].push_back(k);
}

uint64_t Fujimap::getCode(const std::string& value){
  map<string, uint64_t>::const_iterator it = val2code.find(value);
  if (it != val2code.end()){
    return it->second;
  } else {
    uint32_t code = static_cast<uint32_t>(val2code.size());
    val2code[value] = code;
    code2val.push_back(value);
    return code;
  }
}

bool keyEq(const KeyEdge& v1,
	   const KeyEdge& v2){
  for (uint32_t i = 0; i < R; ++i){
    if (v1.v[i]  != v2.v[i]) return false;
  }
  return true; // ignore code
}

int Fujimap::build(){
  for (map<string, uint64_t>::const_iterator it = tmpEdges.begin();
       it != tmpEdges.end(); ++it){
    KeyEdge k(it->first, it->second, seed);
    keyEdges[k.getRaw(0, keyBlockN)].push_back(k);
  }
  tmpEdges.clear();

  size_t totalNum = 0;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    reverse(keyEdges[i].begin(), keyEdges[i].end());
    stable_sort(keyEdges[i].begin(), keyEdges[i].end());
    size_t before = keyEdges[i].size();
    keyEdges[i].erase(unique(keyEdges[i].begin(), keyEdges[i].end(), keyEq), keyEdges[i].end());
    size_t after = keyEdges[i].size();
    if (before != after){
      cerr << before << " " << after << endl;
    }
    totalNum += keyEdges[i].size();
  }

  if (totalNum == 0){
    return 0; // not build
  }

  vector<FujimapBlock> cur;
  for (size_t i = 0; i < keyEdges.size(); ++i){
    FujimapBlock fmb;
    if (fmb.build(keyEdges[i], seed, fpWidth) == -1){
      what_ << "fujimapBlock build error" << endl;
      return -1;
    }
    vector<KeyEdge>().swap(keyEdges[i]);
    cur.push_back(fmb);
  }
  fbs.push_back(cur);
  return 0;
}

size_t Fujimap::getKeyNum() const{
  size_t keyNum = tmpEdges.size();
  for (size_t i = 0; i < keyEdges.size(); ++i){
    keyNum += keyEdges[i].size();
  }

  for (size_t i = 0; i < fbs.size(); ++i){
    for (size_t j = 0; j < fbs[i].size(); ++j){
      keyNum += fbs[i][j].getKeyNum();
    }
  }
  
  return keyNum;

}

std::string Fujimap::getString(const std::string& key) const {
  uint32_t ret = getInteger(key);
  if (ret != NOTFOUND && ret < code2val.size()){
    return code2val[ret];
  } else {
    return string(); // NOTFOUND;
  }
}

uint64_t Fujimap::getInteger(const std::string& key) const {
  map<string, uint64_t>::const_iterator it = tmpEdges.find(key);
  if (it != tmpEdges.end()){
    return it->second;
  }

  for (vector< vector<FujimapBlock> >::const_reverse_iterator it2 = fbs.rbegin();
       it2 != fbs.rend(); ++it2){
    KeyEdge ke(key, 0, seed);
    uint64_t ret = (*it2)[ke.getRaw(0, it2->size())].getVal(ke);
    if (ret != NOTFOUND){
      return ret;
    }
  }

  return NOTFOUND;
}

int Fujimap::load(const char* index){
  ifstream ifs(index);
  if (!ifs){
    what_ << "cannot open " << index << endl;
    return -1;
  }

  uint64_t code2valSize = 0;
  ifs.read((char*)(&code2valSize), sizeof(code2valSize));
  code2val.resize(code2valSize);
  for (uint64_t i = 0; i < code2valSize; ++i){
    loadString(code2val[i], ifs);
  }
  for (size_t i = 0; i < code2val.size(); ++i){
    val2code[code2val[i]] = i;
  }

  ifs.read((char*)(&seed), sizeof(seed));
  ifs.read((char*)(&fpWidth), sizeof(fpWidth));
  ifs.read((char*)(&tmpN), sizeof(tmpN));

  uint64_t keyEdgeSize = 0;
  ifs.read((char*)(&keyEdgeSize), sizeof(keyEdgeSize));
  keyEdges.resize(keyEdgeSize);
  for (uint64_t i = 0; i < keyEdgeSize; ++i){
    uint64_t keyEdgeInSize = 0;
    ifs.read((char*)(&keyEdgeInSize), sizeof(keyEdgeInSize));
    keyEdges[i].resize(keyEdgeInSize);
    for (uint64_t j = 0; j < keyEdgeInSize; ++j){
      keyEdges[i][j].load(ifs);
    }
  }

  uint64_t tmpEdgeSize = 0;
  ifs.read((char*)(&tmpEdgeSize), sizeof(tmpEdgeSize));

  for (uint64_t i = 0; i < tmpEdgeSize; ++i){
    string s;
    loadString(s, ifs);
    uint32_t code = 0;
    ifs.read((char*)(&code), sizeof(uint32_t));
    tmpEdges[s] = code;
  }

  uint64_t fbsSize = 0;
  ifs.read((char*)(&fbsSize), sizeof(fbsSize));
  fbs.resize(fbsSize);
  for (size_t i = 0; i < fbs.size(); ++i){
    uint64_t fbsInSize = 0;
    ifs.read((char*)(&fbsInSize), sizeof(fbsInSize));
    fbs[i].resize(fbsInSize);
    for (size_t j = 0; j < fbs[i].size(); ++j){
      fbs[i][j].load(ifs);
    }
  }
  
  if (!ifs){
    what_ << "read error " << index << endl;
    return -1;
  }
  
  return 0;
}

void Fujimap::saveString(const std::string& s, ofstream& ofs) const{
  uint64_t len =  static_cast<uint64_t>(s.size());
  ofs.write((const char*)(&len), sizeof(len));
  ofs.write(s.c_str(), len);
}

void Fujimap::loadString(std::string& s, ifstream& ifs) const{
  uint64_t len = 0;
  ifs.read((char*)(&len), sizeof(len));
  s.resize(len);
  ifs.read((char*)(&s[0]), len);
}


int Fujimap::save(const char* index){
  ofstream ofs(index);
  if (!ofs){
    what_ << "cannot open " << index << endl;
    return -1;
  }

  uint64_t code2valSize = static_cast<uint64_t>(code2val.size());
  ofs.write((const char*)(&code2valSize), sizeof(uint64_t));
  for (uint64_t i = 0; i < code2valSize; ++i){
    saveString(code2val[i], ofs);
  }

  ofs.write((const char*)(&seed), sizeof(seed));
  ofs.write((const char*)(&fpWidth), sizeof(fpWidth));
  ofs.write((const char*)(&tmpN), sizeof(tmpN));

  uint64_t keyEdgeSize = static_cast<uint64_t>(keyEdges.size());
  ofs.write((const char*)(&keyEdgeSize), sizeof(keyEdgeSize));
  for (uint64_t i = 0; i < keyEdgeSize; ++i){
    uint64_t keyEdgeInSize = keyEdges[i].size();
    ofs.write((const char*)(&keyEdgeInSize), sizeof(keyEdgeInSize));
    for (uint64_t j = 0; j < keyEdgeInSize; ++j){
      keyEdges[i][j].save(ofs);
    }
  }

  uint64_t tmpEdgeSize = static_cast<uint64_t>(tmpEdges.size());
  ofs.write((const char*)(&tmpEdgeSize), sizeof(tmpEdgeSize));
  for (map<string, uint64_t>::const_iterator it = tmpEdges.begin();
       it != tmpEdges.end(); ++it){
    saveString(it->first, ofs);
    ofs.write((const char*)(&it->second), sizeof(it->second));
  }

  uint64_t fbsSize = static_cast<uint64_t>(fbs.size());
  ofs.write((const char*)(&fbsSize), sizeof(fbsSize));
  for (size_t i = 0; i < fbs.size(); ++i){
    uint64_t fbsInSize = fbs[i].size();
    ofs.write((const char*)(&fbsInSize), sizeof(fbsInSize));
    for (size_t j = 0; j < fbs[i].size(); ++j){
      fbs[i][j].save(ofs);
    }
  }
  
  if (!ofs){
    what_ << "write error " << index << endl;
    return -1;
  }
  
  return 0;
}

std::string Fujimap::what() const{
  return what_.str();
}

}
