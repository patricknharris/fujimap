#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

int buildFromFile(const cmdline::parser& p){
  const char* fn = p.get<string>("dic").c_str();
    
  fujimap_tool::Fujimap fm;
  fm.initFP(p.get<int>("fpwidth"));
  fm.initTmpN(p.get<int>("tmpN"));

  ifstream ifs(fn);
  if (!ifs){
    cerr << "Unable to open " << fn << endl;
    return -1;
  }

  map<string, uint32_t> keyValues; // for test
  string line;
  while (getline(ifs, line)){
    size_t p = line.find('\t');
    if (p == string::npos){
      cerr << "Warning: not tab found : " << line << endl;
      continue;
    }
    if (p == 0) continue;
    if (p+1 == line.size()) continue;
    uint32_t val = atoi(line.substr(p+1).c_str());
    string   key = line.substr(0, p);
    fm.setInteger(key, val); // key is searchable immediately
    keyValues[key] = val;
  }

  cerr << "keyNum:" << fm.getKeyNum() << endl;

  int ret = fm.build(); 
  if (ret == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  if (fm.save(p.get<string>("index").c_str()) == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  cerr << fm.getKeyNum() << endl;

  // test
  fujimap_tool::Fujimap fm2;
  if (fm2.load(p.get<string>("index").c_str()) == -1){
    cerr << fm2.what() << endl;
    return -1;
  }

  cerr << "load done." << endl;
  cerr << fm2.getKeyNum() << endl;

  int fnErrorN = 0;
  size_t n = 0;
  for (map<string, uint32_t >::const_iterator it = keyValues.begin();
       it != keyValues.end(); ++it){
    uint32_t ret = fm2.getInteger(it->first);
    if ((++n % 10000) == 0){
      cerr << n << endl;
    }
    if (it->second != ret){
      fnErrorN++;
      //cerr << "Error: " << it->first << endl
      //	   << "correct:" << it->second << " " << " incorrect:" << ret << endl;
      
    }
  }
  cerr << "fnErrorN:" << fnErrorN << endl;
  
  int fpErrorN = 0;
  for (int i = 0; i < 10000; ++i){
    ostringstream os;
    os << i << " " << i+1 << " " << i+2;
    uint32_t ret = fm2.getInteger(os.str());
    if (ret != fujimap_tool::NOTFOUND){
      fpErrorN++;
    }
  }
  cerr << fpErrorN << "/" << 10000 << endl;

  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("dic", 'd', "dictionary", true, "");
  p.add<int>("fpwidth", 'f', "false positive rate 2^{-f} (0 <= f < 31) ", false, fujimap_tool::FPWIDTH);
  p.add<int>("tmpN", 't', "temporarySize", false, fujimap_tool::TMPN);
  p.add<string>("index", 'i', "index", true, "");
  p.add("help", 'h', "print this message");
  p.set_progam_name("fujimap_test");
  
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

  if (buildFromFile(p) == -1){
    return -1;
  }
  return 0;
}
