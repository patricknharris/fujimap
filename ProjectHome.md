# Introduction #

**fujimap** is a library for a space-efficient associative map. It associates a key with a value, and supports "set" and "get" operations, which is fundamental for many applications.
The characteristic of a fujimap is that its working space is surprisingly small; it does not depend on key information, and it only requires a space for storing values by allowing some false positive errors.

For example, if the all values are less than 65536 (can be estored in 16 bit) and false positives are allowed with the probability  2^{-8}, the working space is exactly (16+8) bit per key plus some small overhead whatever keys are. Moreover, fujimap supports these operations in almost constant time. Therefore it can handle very large set of key/values (e.g. 1 billion) on memory.

Fujimap uses several techniques of recent studies in data structures and compression.

## Quick Start ##

To install fujimap, please type the followings.
```
tar xvjf fujimap-x.x.x.tar.bz2
cd fujimap-x.x.x
./ configure
make
sudo make install
```

Command line programs are also provided.

**fujimap**  is a command line program. It supports to build a dictionary file from a key/value file and save it in a specified file.

When you specify a key/value file, and index name, it creates an index from the key/value file and save it in the index. When you specify an index name only, it performs get operations in an interactive mode.

First, you need to prepare a key/value file as follows.
```
key1\tvalue1
key2\tvalue2
...
```
Each line consists of a pair of key and value, and each key is followed by a tab and a  corresponding value.

```
$ fujimap
usage: fujimap --index=string [options] ...
options:
  -i, --index          Index (string)
  -d, --dic            Key/Value file. when "-" is specified it reads from stdin (string [=])
  -f, --fpwidth        False positive rate 2^{-f} (0 <= f < 31)  (int [=0])
  -t, --tmpN           TemporarySize (int [=0])
  -e, --encode         Code encoding  (=binary, gamma) (string [=])
  -w, --workingfile    Working file for temporary data (string [=])
  -l, --logvalue       When specified, store a log of input value
  -s, --stringvalue    When specified, sotre a string of input value
  -h, --help           print this message

$ cat ./dat/input
...
Fuji    4785972
Fuji..  1002
Fuji.jpg        388
Fuji8x  522
FujiBBS 224
FujiCEM 302
FujiChrome      645
FujiColor       1266
FujiCrys        264
...
$ fujimap -d ./dat/input -i ./dat/index -f 10
read 13588391 keys
build done.
keyNum:       13588391
fpLen:        10
encoding:     binary
wsize(bytes): 67343960
bits/key:     52.6506
save done.

$ fujimap -i ./dat/index
load done.
keyNum:       13588391
fpLen:        0
encoding:     binary
wsize(bytes): 67343960
bits/key:     52.6506
>FujiChrome
FOUND:645
>Fuji
FOUND:4785972
>Fujisoba
NOTFOUND:
>
```

You can also use fujimap in your C++ program by including "<fujimap/fujimap.hpp>" and add an option  -lfujimap at a compile time.

To register key/value, use Fujimap::setString() or Fujimap::setInteger()
```
#include <fujimap/fujimap.hpp>
...
fujimap_tool::Fujimap fm;

string key     = string("fujiyama");
uint64_t value = 3775;
fm.setInteger(key.c_str(), key.size(), value;

// string value = string("Japan");
// fm.setString(key.c_str(), key.size(), value.c_str(), value.size()); You can also register a string key
```

To get a value associated with a key, use Fujimap::getString() or Fujimap::getInteger(). If a key is not found, it returns fujimap\_tool::NOTFOUND, and NULL respectively.
```
string key    = string("fujiyama");
uint64_t val  = fm.getInteger(key.c_str(), key.size()); // val == 3775
uint64_t val2 = fm.getInteger("yama", 4);               // val2 == fujimap_tool::NOTFOUND

// size_t retLen = 0;
// const char* sval = fm.getString(key.c_str(), key.size(), retLen); // sval == "Japan" 
```

If the same key is already registered, the old value is replaced with new value.

You can save (load) Fujimap in (from) a file
```
fm.save("index"); // save in a file "index"
fm.load("index"); // load from a file "index
```

## API C++ ##

See [fujimap.hpp](http://code.google.com/p/fujimap/source/browse/trunk/src/fujimap.hpp)

## Example Code ##

```
#include <fujimap/fujimap.hpp>

...
fujimap_tool::Fujimap fm;

fm.initFP(30) // Allow false positives with the probability 2^{-30}

vector<pair<char*, uint64_t> > mountains;
mountains.push_back(make_pair("Fujiyama",     3775));
mountains.push_back(make_pair("Everest",      8844));
mountains.push_back(make_pair("McKinley",     6194));
mountains.push_back(make_pair("Kilima-Njaro", 5895));

for (size_t i = 0; i < mountains.size(); ++i){
  const string&  key = mountains[i].first;
  const uint64_t val = mountains[i].second;
  fm.setInteger(key.c_str(),  key.size(), val, true);  // register and ready to search immediately with some additional space
  assert(fm.getInteger(key.c_str(), key.size()) == val); // the key is searchable immediately
}

string dummyKey = "Aconcagua"; // not registered in fm

if (fm.getInteger(dummyKey.c_str(), dummyKey.size()) != fujimap_tool::NOTFOUND){
  cout << "your unlucky" << endl; // This would happen with the prob. 2^{-30}
}

if (fm.save("myindex") == -1){
  cerr << fm.what() << endl; // error report
  return -1;
}

fujimap_tool::Fujimap fm2;

if (fm2.load("myindex") == -1){
  cerr << fm2.what() << endl; // error report
  return -1;
}

```

Other example codes are found in fujimapMain.cpp fujimapTest.cpp

## Performance Test ##

### Web 1T 5-gram Version 1 ###
  * Use 1gms list as a key/values
  * Number of keyword: 13,588,391
  * Maximum Value 95119665584
  * Average input key length: 64.0 bits
  * -e gamma store in a gamma code
  * Since tx does not store value, we store additional value arrays, and operates by unique id assigned by tx.

Store an approximate value (a log of input value)
|Option                |Size (bits/key)|Time (sec.)|
|:---------------------|:--------------|:----------|
|(Raw)                 |84.0           |           |
|std::map              |573.4          |38.0       |
|tx                    |35.4 (29.4+6)  |30.0       |
|fujimap -l            |6.9            |45.5       |
|fujimap -l -e gamma   |4.4            |66.5       |

Store an exact value
|Option                |Size (bits/key)|Time (sec.)|
|:---------------------|:--------------|:----------|
|(Raw)                 |111.0          |           |
|std::map              |604.4          |39.5       |
|tx                    |66.4 (29.4+37) |30.5       |
|fujimap               |39.6           |41.7       |
|fujimap -e gamma      |22.1           |204.1      |


### Random Key and Value ###
  * Generate numbers from 0 to 10^8-1, and use this as key and values
  * Number of Keywords: 10^8 (100M)
  * This can be reproduced by the following command
```
 fujimapPerformanceTest 100000000
```

|Method  | Build Time (sec.) | Lookup Time (sec.) |
|:-------|:------------------|:-------------------|
|fm      | 29.5              | 5.5 x 10^{-8}      |
|std::map| 16.0              | 1.1 x 10^{-7}      |

## Note ##

Here, we use the notation following the Bloom filter; positive corresponds to the filtered out items, and  "false positive" refers to the item which should be filtered out, are misclassified as registered items.