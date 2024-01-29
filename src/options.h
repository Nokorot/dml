#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

struct Options {
     std::string cacheDir; 

     bool ignoreCache;
     bool recent;

     int historyLength = 5;
};

#endif
