#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include "fujimap.hpp"

using namespace std;

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

static int N = 1000000;

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void run_fujimap(){
  fujimap_tool::Fujimap fm;
  fm.initFP(0);
  fm.initTmpN(0);

  char buf[100];
  double t1 = gettimeofday_sec();
  for (int i = 0; i < N; ++i){
    snprintf(buf, 100, "%d", i);
    fm.setIntegerTemporary(string(buf), i);
  }
  fm.build();
  double t2 = gettimeofday_sec();
  fprintf(stderr, "%f (%f)\n", t2 - t1, (t2 - t1) / N);

  int dummy = 0;
  for (int i = 0; i < N; ++i){
    snprintf(buf, 100, "%d", i);
    dummy += fm.getInteger(string(buf));
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "%f (%f)\n", t3 - t2, (t3 - t2) / N);

  if (dummy == 77777){
    fprintf(stderr, "you're lucky!");
  }

}

void run_map(){
  map<string, int> m;

  char buf[100];
  double t1 = gettimeofday_sec();
  for (int i = 0; i < N; ++i){
    snprintf(buf, 100, "%d", i);
    m[string(buf)] = i;
  }
  double t2 = gettimeofday_sec();
  fprintf(stderr, "%f (%f)\n", t2 - t1, (t2 - t1) / N);

  int dummy = 0;
  for (int i = 0; i < N; ++i){
    snprintf(buf, 100, "%d", i);
    dummy += m[string(buf)];
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "%f (%f)\n", t3 - t2, (t3 - t2) / N);

  if (dummy == 77777){
    fprintf(stderr, "you're lucky!");
  }

  
}


int main(int argc, char* argv[]){
  run_fujimap();
  run_map();
  return 0;
}
