/*******************************************************************************
MIT License

Copyright (c) 2018 SURFsara BV

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#pragma once
#include <string>
#include <iostream>
//@todo move to different project when it will be used in other tools as well

namespace surfsara
{
  namespace util
  {
    inline std::string readPassord(std::istream & ist = std::cin);

    template<typename T>
    inline T fromString(const std::string & str);
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



