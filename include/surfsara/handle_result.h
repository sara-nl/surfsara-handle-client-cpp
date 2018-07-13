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
#include <iostream>
#include <surfsara/ast.h>
#include <surfsara/curl.h>

namespace surfsara
{
  namespace handle
  {
    using Node = ::surfsara::ast::Node;

    struct Result
    {
      ::surfsara::curl::Result curlResult;
      long        handleCode;
      bool        jsonDecodeError;
      bool        success;
      std::string handle;
      surfsara::ast::Node data;

      Result() :
        handleCode(0),
        jsonDecodeError(false),
        success(false) {}
    };

    /* helper functions */
    inline const char * responseCode2string(long code);
    inline ::std::ostream & operator<<(::std::ostream & ost, const Result & res);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
namespace surfsara
{
  namespace handle
  {
    inline ::std::ostream & operator<<(::std::ostream & ost, const Result & res)
    {
      ost << res.curlResult << std::endl
          << "hdl resp:  " << res.handleCode << " (" << responseCode2string(res.handleCode) << ")" << std::endl
          << "success:   " << res.success << std::endl
          << "jsonError: " << res.jsonDecodeError << std::endl
          << "handle:    " << res.handle;
      return ost;
    }

    inline const char * responseCode2string(long code)
    {
      switch(code)
      {
      case 1: return "Success";
      case 2: return "An unexpected error on the server";
      case 100: return "Handle not found";
      case 101: return "Handle already exists";
      case 102: return "Invalid handle";
      case 200: return "Values not found";
      case 201: return "Value already exists";
      case 202: return "Invalid value";
      case 301: return "Server not responsible for handle";
      case 402: return "Authentication needed";
      };

      if(code > 403 && code < 410)
      {
        return "Other authentication errors";
      }
      else
      {
        return "Unknown";
      }
    }

  }
}

