#pragma once
#include <iostream>
#include <ctype.h>

class WordCounter
{
public:
  WordCounter();
  inline std::size_t read(const char * buff, std::size_t n);
  inline std::size_t read(std::istream & ist);
  inline std::size_t getNumWords() const;
private:
  bool inWordState;
  std::size_t numWords;
};

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline WordCounter::WordCounter() : inWordState(false), numWords(0u) {
}

inline std::size_t WordCounter::read(const char * buff, std::size_t n) {
  for(std::size_t i = 0; i < n; i++) {
    if(inWordState) {
      if(isspace(buff[i])) {
        numWords++;
        inWordState = false;
      }
    }
    else {
      if(!isspace(buff[i])) {
        inWordState = true;
      }
    }
  }
  return getNumWords();
}

inline std::size_t WordCounter::read(std::istream & ist) {
  char buff[10];
  while(ist.good()) {
    ist.read(buff, sizeof(buff));
    read(buff, ist.gcount());
  }
  return getNumWords();
}

inline std::size_t WordCounter::getNumWords() const {
  return numWords + (inWordState ? 1 : 0);
}
