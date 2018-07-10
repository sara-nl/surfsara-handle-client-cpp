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
#include "curl_opt.h"
#include <curl/curl.h>
#include <vector>
#include <exception>
#include <sstream>
#include <initializer_list>

namespace surfsara
{
  namespace curl
  {
    struct Result
    {
    public:
      long httpCode;
      CURLcode curlCode;
      bool success;
      std::string body;
    };

    class Error : public std::exception
    {
    public:
      Error(const Result & _res);
      virtual const char* what() const noexcept override
      {
        return msg.c_str();
      }
    private:
      std::string msg;
    };

    class Curl
    {
    public:
      using InitializerList = std::initializer_list<std::shared_ptr<details::BasicCurlOpt>>;
      Curl(const InitializerList & options);
      Curl(const std::vector<std::shared_ptr<details::BasicCurlOpt>> & options);
      ~Curl();
      inline Result request();
      static inline const char * httpCode2string(long);
    private:
      static size_t write(char *ptr, size_t size, size_t nmemb, void *userdata);
      CURL *curl;
      std::vector<std::shared_ptr<details::BasicCurlOpt>> optSetter;
      std::string buffer;
    };
  }
}

///////////////////////////////////////////
namespace surfsara
{
  namespace curl
  {
    inline Error::Error(const Result & _res)
    {
      std::stringstream serr;
      serr << "http code: " << _res.httpCode << "("
           << Curl::httpCode2string(_res.httpCode) << ")" << std::endl
           << "curl code: " << _res.curlCode
           << " (" << curl_easy_strerror(_res.curlCode) << ")" << std::endl
           << "body       " << _res.body << std::endl;
      msg = serr.str();
    }
    
    inline Curl::Curl(const InitializerList & options) :
      optSetter(options.begin(), options.end())
    {
      curl = curl_easy_init();
      for(auto setter : optSetter) {
        setter->set(curl);
      }
    }

    inline Curl::Curl(const std::vector<std::shared_ptr<details::BasicCurlOpt>> & options) :
      optSetter(options)
    {
      curl = curl_easy_init();
      for(auto setter : optSetter) {
        setter->set(curl);
      }
    }

    inline Curl::~Curl()
    {
      curl_easy_cleanup(curl);
    }

    inline Result Curl::request()
    {
      Result res;
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Curl::write);
      res.curlCode = curl_easy_perform(curl);
      res.httpCode = 0;
      res.success = false;
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &res.httpCode);
      if (res.httpCode == 200 && res.curlCode != CURLE_ABORTED_BY_CALLBACK)
      {
        res.success = true;
      }
      return res;
    }

    size_t Curl::write(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
      auto result = static_cast<std::string*>(userdata);
      result->append(ptr, ptr + nmemb);
    }

    inline const char * Curl::httpCode2string(long code)
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
  }
}
