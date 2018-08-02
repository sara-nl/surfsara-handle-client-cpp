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

