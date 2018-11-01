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
#include "curl_opt.h"
#include "curl_util.h"
#include <curl/curl.h>
#include <vector>
#include <exception>
#include <sstream>
#include <initializer_list>
#include <iostream>

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
      Result() : httpCode(0), curlCode(CURLE_OK), success(false) {}
    };
    ::std::ostream & operator<<(::std::ostream & ost, const Result & res);
   
    class Curl
    {
    public:
      using InitializerList = std::initializer_list<std::shared_ptr<BasicCurlOpt>>;
      Curl(const InitializerList & options);
      Curl(const std::vector<std::shared_ptr<BasicCurlOpt>> & options);
      ~Curl();
      inline Result request();
    private:
      static size_t write(char *ptr, size_t size, size_t nmemb, void *userdata);
      CURL *curl;
      std::vector<std::shared_ptr<BasicCurlOpt>> optSetter;
      std::string buffer;
    };
  }
}

///////////////////////////////////////////
namespace surfsara
{
  namespace curl
  {
    ::std::ostream & operator<<(::std::ostream & ost, const Result & res)
    {
      ost << "http code: " << res.httpCode << " (" << httpCode2string(res.httpCode) << ")" << std::endl
          << "curl code: " << res.curlCode << " (" << curlCode2string(res.curlCode) << ")" << std::endl
          << "success:   " << res.success;
      if(res.success)
      {
        ost << std::endl << "body       " << res.body;
      }
      return ost;
    }

    inline Curl::Curl(const InitializerList & options) :
      optSetter(options.begin(), options.end())
    {
      curl = curl_easy_init();
      for(auto setter : optSetter)
      {
        if(setter)
        {
          setter->set(curl);
        }
      }
    }

    inline Curl::Curl(const std::vector<std::shared_ptr<BasicCurlOpt>> & options) :
      optSetter(options)
    {
      curl = curl_easy_init();
      for(auto setter : optSetter)
      {
        if(setter)
        {
          setter->set(curl);
        }
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
      if (httpCodeIsSuccess(res.httpCode) && res.curlCode != CURLE_ABORTED_BY_CALLBACK)
      {
        res.success = true;
      }
      return res;
    }

    size_t Curl::write(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
      auto result = static_cast<std::string*>(userdata);
      result->append(ptr, ptr + nmemb);
      return nmemb;
    }
  }  // curl
} // surfsara
