#ifndef KEYFILE_HPP__
#define KEYFILE_HPP__

#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

namespace fujimap_tool{


class KeyFile{
  enum {
    BLOCKSIZE = 4096
  };

public:
  KeyFile();
  ~KeyFile();

  int initWorkingFile(const char* fn);
  void initMaxID(const uint64_t maxID);
  int clear();
  int write(const uint64_t id, const char* key, const size_t klen,
	    const uint64_t value);
  int read(const uint64_t id, std::vector<std::pair<std::string, uint64_t> >& kvs);
  size_t getNum() const;

private:
  std::string fns;
  static void writeUint64(std::vector<char>& v, const uint64_t x);
  static uint64_t readUint64(std::vector<char>::const_iterator& it);
  std::vector< std::vector<std::pair<std::string, uint64_t> > > buffers;
  std::vector<uint64_t> nextPointers;
  std::vector<uint64_t> firstPointers;
  size_t num;
  uint64_t maxID;

};

}

#endif // KEY_FILE_HPP__

