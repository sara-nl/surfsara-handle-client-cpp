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

    class Curl
    {
    public:
      using InitializerList = std::initializer_list<std::shared_ptr<details::BasicCurlOpt>>;
      Curl(const InitializerList & options);
      Curl(const std::vector<std::shared_ptr<details::BasicCurlOpt>> & options);
      ~Curl();
      inline Result request();
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

    inline Curl::~Curl() {
      curl_easy_cleanup(curl);
    }

    inline Result Curl::request() {
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

    size_t Curl::write(char *ptr, size_t size, size_t nmemb, void *userdata) {
      auto result = static_cast<std::string*>(userdata);
      result->append(ptr, ptr + nmemb);
    }
  }
}
