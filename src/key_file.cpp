#include <iostream>
#include <cassert>
#include "key_file.hpp"

using namespace std;

namespace fujimap_tool{

static const char* TMPKF = "tmp.kf";


KeyFile::KeyFile() :  fns(TMPKF), num(0), maxID(0){
  initWorkingFile(fns.c_str());
}

KeyFile::~KeyFile(){
}

size_t KeyFile::getNum() const{
  return num;
}

int KeyFile::clear(){
  num = 0;
  buffers.clear();
  nextPointers.clear();
  firstPointers.clear();
  buffers.resize(maxID);
  nextPointers.resize(maxID);
  firstPointers.resize(maxID);

  ofstream ofs(fns.c_str());
  ofs.write((const char*)(&maxID), sizeof(uint64_t));
  if (!ofs){
    return -1;
  }

  return 0;
}



int KeyFile::initWorkingFile(const char* fn){
  fns = fn;
  ofstream ofs(fns.c_str());
  ofs.write((const char*)(&maxID), sizeof(uint64_t));
  if (!ofs){
    return -1;
  }
  return 0;
}
 
void KeyFile::initMaxID(const uint64_t maxID_){
  maxID = maxID_;
  buffers.resize(maxID);
  nextPointers.resize(maxID);
  firstPointers.resize(maxID);
}


int KeyFile::write(const uint64_t id, const char* kbuf, const size_t klen, 
		     const uint64_t value){
  assert(id < maxID);
  vector<pair<string, uint64_t> >& v(buffers[id]);
  string s;
  s.assign(kbuf, klen);
  v.push_back(make_pair(s, value));

  if (v.size() >= BLOCKSIZE){
    fstream ofs(fns.c_str(),  std::ios::in | std::ios::out);
    ofs.seekp(0, ios::end);
    uint64_t curPos = ofs.tellp();
    if (firstPointers[id] == 0){
      firstPointers[id] = curPos;
    } else {
      ofs.seekp(nextPointers[id], ios::beg);
      ofs.write((const char*)(&curPos), sizeof(uint64_t));
      if (!ofs){
	return -1;
      }
      ofs.seekp(curPos, ios::beg);
      if (!ofs){
	return -1;
      }
    }

    uint64_t vnum = static_cast<uint64_t>(v.size());
    ofs.write((const char*)(&vnum), sizeof(uint64_t));
    for (size_t i = 0; i < v.size(); ++i){
      uint64_t klen = v[i].first.size();
      ofs.write((const char*)(&klen), sizeof(uint64_t));
      ofs.write((const char*)(v[i].first.c_str()), klen);
      ofs.write((const char*)(&v[i].second), sizeof(uint64_t));
    }

    nextPointers[id] = static_cast<uint64_t>(ofs.tellp());
    uint64_t dummy = 0;
    ofs.write((const char*)(&dummy), sizeof(uint64_t));
    if (!ofs){
      return -1;
    }
    
    vector<pair<string, uint64_t> >().swap(v);
  }
  ++num;
  return 0;
}

int KeyFile::read(const uint64_t id, vector<pair<std::string, uint64_t> >& kvs){
  uint64_t readPos = firstPointers[id];
  ifstream ifs(fns.c_str());
  size_t blockNum = 0;
  while (readPos != 0){
    blockNum++;
    ifs.seekg(readPos, ios::beg);
    uint64_t vnum = 0;
    ifs.read((char*)(&vnum), sizeof(uint64_t));
    if (!ifs) return -1;
    for (uint64_t i = 0; i < vnum; ++i){
      uint64_t klen = 0;
      ifs.read((char*)(&klen), sizeof(uint64_t));
      if (!ifs) return -1;
      std::string s;
      s.resize(klen);
      ifs.read((char*)(&s[0]), klen);
      if (!ifs) return -1;
      uint64_t value = 0;
      ifs.read((char*)(&value), sizeof(uint64_t));
      if (!ifs) return -1;
      kvs.push_back(make_pair(s, value));
    }

    ifs.read((char*)(&readPos), sizeof(uint64_t));
    if (!ifs){
      cerr << "read error" << endl;
      return -1;
    }

    if (!ifs) return -1;
  }

  for (size_t i = 0; i < buffers[id].size(); ++i){
    kvs.push_back(buffers[id][i]);
  }
  return 0;
}





}
