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
#include <curl/curl.h>

namespace surfsara
{
  namespace curl
  {
    inline const char * httpCode2string(long);
    inline const char * curlCode2string(CURLcode);
    inline bool httpCodeIsSuccess(long code);
  }
}

namespace surfsara
{
  namespace curl
  {
    inline const char * httpCode2string(long code)
    {
      switch(code)
      {
      case 100: return "Continue";
      case 101: return "Switching Protocols";
      case 200: return "OK";
      case 201: return "Created";
      case 202: return "Accepted";
      case 203: return "Non-Authoritative Information";
      case 204: return "No Content";
      case 205: return "Reset Content";
      case 206: return "Partial Content";
      case 300: return "Multiple Choices";
      case 301: return "Moved Permanently";
      case 302: return "Found";
      case 303: return "See Other";
      case 304: return "Not Modified";
      case 306: return "Switch Proxy";
      case 307: return "Temporary Redirect";
      case 308: return "Resume Incomplete";
      case 400: return "Bad Request";
      case 401: return "Unauthorized";
      case 402: return "Payment Required";
      case 403: return "Forbidden";
      case 404: return "Not Found";
      case 405: return "Method Not Allowed";
      case 406: return "Not Acceptable";
      case 407: return "Proxy Authentication Required";
      case 408: return "Request Timeout";
      case 409: return "Conflict";
      case 410: return "Gone";
      case 411: return "Length Required";
      case 412: return "Precondition Failed";
      case 413: return "Request Entity Too Large";
      case 414: return "Request-URI Too Long";
      case 415: return "Unsupported Media Type";
      case 416: return "Requested Range Not Satisfiable";
      case 417: return "Expectation Failed";
      case 500: return "Internal Server Error";
      case 501: return "Not Implemented";
      case 502: return "Bad Gateway";
      case 503: return "Service Unavailable";
      case 504: return "Gateway Timeout";
      case 505: return "HTTP Version Not Supported";
      case 511: return "Network Authentication Required";
      default: return "Unknown Error";
      }
    }
    
    inline const char * curlCode2string(CURLcode code)
    {
      return curl_easy_strerror(code);
    }
    
    inline bool httpCodeIsSuccess(long code)
    {
      return code >= 200 && code < 300;
    }

  } // curl
} // surfsara
