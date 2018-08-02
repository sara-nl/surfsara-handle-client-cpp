/*
Copyright 2018, SURFsara
Author Stefan Wolfsheimer


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once
#include <string>
#include <iostream>

namespace surfsara
{
  namespace util
  {
    inline std::string readPassord(std::istream & ist = std::cin);

    template<typename T>
    inline T fromString(const std::string & str);

    inline std::string joinPath(const std::string & s1, const std::string & s2);
  }
}

////////////////////////////////////////////////////////////////////
//
// implementation
//
////////////////////////////////////////////////////////////////////
#include <termios.h>
#include <unistd.h>
#include <sstream>

inline std::string  surfsara::util::readPassord(std::istream & ist)
{
  bool outputEnabled = true;
  std::string password;
  if(&ist == &std::cin)
  {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    outputEnabled = (tty.c_lflag & ECHO);
    if(outputEnabled)
    {
      tty.c_lflag &= ~ECHO;
      tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    }
  }
  ist >> password;
  if(&ist == &std::cin)
  {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if(outputEnabled)
    {
      tty.c_lflag |= ECHO;    
      tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    }
  }
  return password;
}

template<typename T>
inline T surfsara::util::fromString(const std::string & str)
{
  std::stringstream stream(str);
  T value;
  stream >> value;
  if(stream.fail() || !stream.eof()) 
  {
    throw std::logic_error(std::string("could not convert '") + str + ("' to ") + typeid(T).name());
  }
  return value;
}

inline std::string surfsara::util::joinPath(const std::string & s1, const std::string & s2)
{
  std::size_t p = s1.size();
  while(p > 0 && s1[p-1] == '/')
  {
    p--;
  }
  std::size_t q = 0;
  while(q < s2.size() && s2[q] == '/')
  {
    q++;
  }
  std::string ret(s1.begin(), s1.begin() + p);
  ret.append("/");
  ret.insert(ret.end(), s2.begin() + q, s2.end());
  return ret;
}

