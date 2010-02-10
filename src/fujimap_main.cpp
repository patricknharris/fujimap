#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

int buildFromFile(const cmdline::parser& p){
  const char* fn = p.get<string>("dic").c_str();
    
  fujimap_tool::Fujimap fm;
  fm.initFP(p.get<int>("fpwidth"));
  fm.initTmpN(p.get<int>("tmpN"));

  fujimap_tool::Fujimap fmlen;
  fmlen.initFP(p.get<int>("fpwidth"));
  fmlen.initTmpN(p.get<int>("tmpN"));

  ifstream ifs(fn);
  if (!ifs){
    cerr << "Unable to open " << fn << endl;
    return -1;
  }

  string line;
  while (getline(ifs, line)){
    size_t p = line.find_last_of('\t');
    if (p == string::npos){
      cerr << "Warning: not tab found : " << line << endl;
      continue;
    }
    if (p == 0 || p+1 == line.size()) continue; // no key or no value
    uint64_t val = strtoll(line.substr(p+1).c_str(), NULL, 10);
    string   key = line.substr(0, p);

    fm.setIntegerTemporary(key, val);

    uint64_t len = fujimap_tool::FujimapBlock::log2(val);
    fmlen.setIntegerTemporary(key, len); 
  }
  cerr << "keyNum:" << fm.getKeyNum() << endl;

  int ret = fmlen.build(); 
  if (ret == -1){
    return -1;
  }
  cerr << "build done." << endl;

  if (fmlen.save(p.get<string>("index").c_str()) == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  cerr << "save done." << endl;
  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("dic", 'd', "dictionary", false, "");
  p.add<string>("index", 'i', "index", true, "");
  p.add<int>("fpwidth", 'f', "false positive rate 2^{-f} (0 <= f < 31) ", false, 0);
  p.add<int>("tmpN", 't', "temporarySize", false, 1000000);
  p.add("help", 'h', "print this message");
  p.set_progam_name("fujimap");
  
  bool parseOK = p.parse(argc, argv);
  if (argc == 1 || p.exist("help")){
    cerr << p.usage();
    return -1;
  }

  if (!parseOK){
    cerr << p.error() << endl
	 << p.usage();
    return -1;
  }

  if (p.exist("dic")){
    if (buildFromFile(p) == -1){
      return -1;
    }
  } else {
    fujimap_tool::Fujimap fm;
    if (fm.load(p.get<string>("index").c_str()) == -1){
      cerr << fm.what() << endl;
      return -1;
    }

    cout << "load done. " << fm.getKeyNum() << " keys." << endl;

    string key;
    while (getline(cin, key)){
      cout << ">";
      string val = fm.getString(key);
      cout << "val:" << val << endl;
      uint32_t code = fm.getInteger(key);
      cout << "code:" << code << endl;
    }
  }

  return 0;
}
