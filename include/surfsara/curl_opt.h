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
#include <memory>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <cstring>

namespace surfsara
{
  namespace curl
  {
    class Curl;

    namespace details
    {
      class BasicCurlOpt;

      template<typename T>
      struct OptConverter;
    }
    static std::shared_ptr<details::BasicCurlOpt> Url(const std::string & url);
    static std::shared_ptr<details::BasicCurlOpt> Port(long port);
    static std::shared_ptr<details::BasicCurlOpt> Data(const std::string & data);
    static std::shared_ptr<details::BasicCurlOpt> Header(const std::initializer_list<std::string> & _headers);
    static std::shared_ptr<details::BasicCurlOpt> Header(const std::vector<std::string> & _headers);
    static std::shared_ptr<details::BasicCurlOpt> Verbose(bool verbose);
  }
}

/////////////////////////////////////////////////////////////
//
// details
//
/////////////////////////////////////////////////////////////
namespace surfsara
{
  namespace curl
  {
    namespace details
    {
      template<typename T>
      struct OptConverter
      {
        static T convert(const T & value) { return value; }
      };

      template<>
      struct OptConverter<const std::string>
      {
        static const char * convert(const std::string& value) { return value.c_str(); }
      };

      template<>
      struct OptConverter<std::string>
      {
        static const char * convert(const std::string& value) { return value.c_str(); }
      };

      class BasicCurlOpt
      {
      public:
        virtual ~BasicCurlOpt() {}
        virtual CURLcode set(CURL *curl) const = 0;
      };

      ///// CurlOpt /////
      template<typename T, CURLoption OPTION, typename CONV = OptConverter<T> >
      class CurlOpt : public BasicCurlOpt
      {
      public:
        CurlOpt(const T & _value) : value(_value) {}
        CURLcode set(CURL *curl) const override {
          curl_easy_setopt(curl, OPTION, CONV::convert(value));
        }
      protected:
        T value;
      };

      ///// DataBuffer /////
      class DataBuffer : public BasicCurlOpt
      {
      public:
        DataBuffer(const std::string & _buffer) : buffer(_buffer), uploaded(0) {}

        virtual CURLcode set(CURL *curl) const override
        {
          curl_easy_setopt(curl, CURLOPT_READDATA, this);
          curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
          curl_easy_setopt(curl, CURLOPT_INFILESIZE, buffer.size());
          return curl_easy_setopt(curl, CURLOPT_READFUNCTION, &DataBuffer::read); 
        }

      private:
        static std::size_t read(void *ptr, size_t size, size_t nmemb, void *data)
        {
          auto self = static_cast<DataBuffer*>(data);
          size_t left = self->buffer.size() - self->uploaded;
          size_t max_chunk = size * nmemb;
          size_t retcode = left < max_chunk ? left : max_chunk;
          std::memcpy(ptr, self->buffer.c_str() + self->uploaded, retcode);
          self->uploaded += retcode;
          return retcode;
        }

        std::string buffer;
        size_t uploaded;
      };

      class HeaderList : public BasicCurlOpt
      {
      public:
        HeaderList(const std::initializer_list<std::string> & _headers) : headers(_headers){}
        HeaderList(const std::vector<std::string> & _headers) : headers(_headers){}

        virtual CURLcode set(CURL *curl) const override
        {
          struct curl_slist *chunk = NULL;
          for(auto line : headers) {
            chunk = curl_slist_append(chunk, line.c_str());
          }
          return curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        }

      private:
        std::vector<std::string> headers;
      };
      
    } // details

    std::shared_ptr<details::BasicCurlOpt> Url(const std::string & url) {
      return std::make_shared<details::CurlOpt<std::string, CURLOPT_URL>>(url);
    }

    std::shared_ptr<details::BasicCurlOpt> Port(long port) {
      return std::make_shared<details::CurlOpt<long, CURLOPT_PORT>>(port);
    }

    std::shared_ptr<details::BasicCurlOpt> Data(const std::string & data) {
      return std::make_shared<details::DataBuffer>(data);
    }

    std::shared_ptr<details::BasicCurlOpt> Header(const std::initializer_list<std::string> & _headers) {
      return std::make_shared<details::HeaderList>(_headers);
    }
    
    std::shared_ptr<details::BasicCurlOpt> Header(const std::vector<std::string> & _headers) {
      return std::make_shared<details::HeaderList>(_headers);
    }

    std::shared_ptr<details::BasicCurlOpt> Verbose(bool verbose) {
      return std::make_shared<details::CurlOpt<long, CURLOPT_VERBOSE>>(verbose ? 1L : 0L);
    }
  }
}

