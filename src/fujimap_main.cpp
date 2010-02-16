#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

int buildFromFile(cmdline::parser& p){
  istream *pis=&cin;
  ifstream ifs;
  if (p.get<string>("dic") != "-"){
    ifs.open(p.get<string>("dic").c_str());
    if (!ifs){
      cerr << "Unable to open " << p.get<string>("dic") << endl;
      return -1;
    }

    pis = static_cast<istream*>(&ifs);
  }

  istream &is=*pis;
  fujimap_tool::Fujimap fm;
  uint64_t fpWidth = 0;
  if (p.exist("fpwidth")){
    fm.initFP(p.get<int>("fpwidth"));
    fpWidth = p.get<int>("fpwidth");
  }
  
  if (p.exist("tmpN")){
    fm.initTmpN(p.get<int>("tmpN"));
  }

  string line;
  while (getline(is, line)){
    size_t p = line.find_last_of('\t');
    if (p == string::npos){
      cerr << "Warning: not tab found : " << line << endl;
      continue;
    }
    if (p == 0 || p+1 == line.size()) continue; // no key or no value
    uint64_t val = strtoll(line.substr(p+1).c_str(), NULL, 10);
    fm.setInteger(line.c_str(), p, val, false);
  }
  cerr << "keyNum:" << fm.getKeyNum() << endl
       << "fpWidth:" << fpWidth << endl;


  int ret = fm.build(); 
  if (ret == -1){
    return -1;
  }
  cerr << "build done." << endl;

  if (fm.save(p.get<string>("index").c_str()) == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  cerr << "save done." << endl;
  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("dic", 'd', "dictionary file. when \"-\" is specificed it reads from stdin", false);
  p.add<string>("index", 'i', "index", true, "");
  p.add<int>("fpwidth", 'f', "false positive rate 2^{-f} (0 <= f < 31) ", false);
  p.add<int>("tmpN", 't', "temporarySize", false);
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
    for (;;){
      cout << ">" << flush;
      if (!getline(cin, key)){
	break;
      }

      size_t len = 0;
      const char* sval = fm.getString(key.c_str(), key.size(), len);
      if (sval != NULL){
	cout << "sval:" << sval << endl;
      }
      uint64_t code = fm.getInteger(key.c_str(), key.size());
      if (code == fujimap_tool::NOTFOUND){
	cout << "notfound" << endl;
      } else {
	cout << "code:" << code << endl;
      }
    }
  }

  return 0;
}
