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
    class BasicCurlOpt;

    namespace details
    {
      template<typename T>
      struct OptConverter;
    }
    static std::shared_ptr<BasicCurlOpt> Url(const std::string & url,
                                             const std::vector<std::pair<std::string, std::string>> & parameters={});
    static std::shared_ptr<BasicCurlOpt> Port(long port);
    static std::shared_ptr<BasicCurlOpt> SslPem(const std::string & _certFile,
                                                const std::string & _keyFile,
                                                bool                _insecure = true,
                                                const std::string & _passphrase = "",
                                                const std::string & _caCert = "",
                                                const std::string & _caCertPath = "");
    std::shared_ptr<BasicCurlOpt> HttpAuth(const std::string & _user,
                                           const std::string & _password,
                                           bool                _insecure = true,
                                           const std::string & _caCert = "",
                                           const std::string & _caCertPath = "");
    static std::shared_ptr<BasicCurlOpt> Delete();
    static std::shared_ptr<BasicCurlOpt> Data(const std::string & data);
    static std::shared_ptr<BasicCurlOpt> Header(const std::initializer_list<std::string> & _headers);
    static std::shared_ptr<BasicCurlOpt> Header(const std::vector<std::string> & _headers);
    static std::shared_ptr<BasicCurlOpt> Session(CURLSH * share);
    static std::shared_ptr<BasicCurlOpt> CacheSessionId(bool do_cache);
    static std::shared_ptr<BasicCurlOpt> Verbose(bool verbose);
  }
}

#include <iostream>
/////////////////////////////////////////////////////////////
//
// details
//
/////////////////////////////////////////////////////////////
namespace surfsara
{
  namespace curl
  {
    class BasicCurlOpt
    {
    public:
      virtual ~BasicCurlOpt() {}
      virtual CURLcode set(CURL *curl) const = 0;
    };
    
    namespace details
    {
      template<typename T>
      struct OptConverter
      {
        static T convert(const T & value) {
          return value;
        }
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

      ///// CurlOpt /////
      template<typename T, CURLoption OPTION, typename CONV = OptConverter<T> >
      class CurlOpt : public BasicCurlOpt
      {
      public:
        CurlOpt(const T & _value) : value(_value) {}
        CURLcode set(CURL *curl) const override {
          return curl_easy_setopt(curl, OPTION, CONV::convert(value));
        }
      protected:
        T value;
      };

      class Url : public BasicCurlOpt
      {
      public:
        Url(const std::string & _url,
            const std::vector<std::pair<std::string, std::string>> & _parameters) :
          url(_url), parameters(_parameters)
        {
        }
        
        CURLcode set(CURL *curl) const override
        {
          std::string query;
          for(auto p : parameters)
          {
            if(query.empty())
            {
              query += "?";
            }
            else
            {
              query += "&";
            }
            char * key = curl_easy_escape(curl, p.first.c_str(), p.first.size());
            char * value = curl_easy_escape(curl, p.second.c_str(), p.second.size());
            query += key;
            query += "=";
            query += value;
            if(key)
            {
              curl_free(key);
            }
            if(value)
            {
              curl_free(value);
            }
          }
          std::string tmp = url + query;
          return curl_easy_setopt(curl, CURLOPT_URL, tmp.c_str());
        }
      private:
        std::string url;
        std::vector<std::pair<std::string, std::string>> parameters;
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

      ///////////////// SSL /////////////////
      class SslPem : public BasicCurlOpt
      {
      public:
        SslPem(const std::string & _certFile,
               const std::string & _keyFile,
               bool                _insecure,
               const std::string & _passphrase,
               const std::string & _caCert,
               const std::string & _caCertPath)
          : certFile(_certFile),
            keyFile(_keyFile),
            insecure(_insecure),
            passphrase(_passphrase),
            caCertFile(_caCert),
            caCertPath(_caCertPath)
        {
        }

        virtual CURLcode set(CURL *curl) const override
        {
          CURLcode c;
          CURLcode ret = CURLE_OK;
          c = curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
          if(c != CURLE_OK) ret = c;
          c = curl_easy_setopt(curl, CURLOPT_SSLCERT, certFile.c_str());
          if(c != CURLE_OK) ret = c;
          c = curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
          if(c != CURLE_OK) ret = c;
          if(!keyFile.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFile.c_str());
            if(c != CURLE_OK) ret = c;
          }
          if(!passphrase.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_KEYPASSWD, passphrase.c_str());
            if(c != CURLE_OK) ret = c;
          }
          if(insecure)
          {
            c = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            if(c != CURLE_OK) ret = c;
          }
          else
          {
            c = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            if(c != CURLE_OK) ret = c;
          }
          if(!caCertFile.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_CAINFO, caCertFile.c_str());
            if(c != CURLE_OK) ret = c;
          }
          if(!caCertPath.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_CAPATH, caCertPath.c_str());
            if(c != CURLE_OK) ret = c;
          }
          return ret;
        }
      private:
        std::string certFile;
        std::string keyFile;
        std::string passphrase;
        std::string caCertFile;
        std::string caCertPath;
        bool insecure;
      };
      
      ///////////////// SSL /////////////////
      class HttpAuth : public BasicCurlOpt
      {
      public:
        HttpAuth(const std::string & _user,
                 const std::string & _password,
                 bool                _insecure,
                 const std::string & _caCertFile,
                 const std::string & _caCertPath) : user(_user),
                                                    password(_password),
                                                    caCertFile(_caCertFile),
                                                    caCertPath(_caCertPath),
                                                    insecure(_insecure) {}
        virtual CURLcode set(CURL *curl) const override
        {
          CURLcode c;
          CURLcode ret = CURLE_OK;
          if(insecure)
          {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
          }
          else
          {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
          }
          if(!caCertFile.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_CAINFO, caCertFile.c_str());
            if(c != CURLE_OK) ret = c;
          }
          if(!caCertPath.empty())
          {
            c = curl_easy_setopt(curl, CURLOPT_CAPATH, caCertPath.c_str());
            if(c != CURLE_OK) ret = c;
          }
          std::string tmp = user;
          if(!password.empty())
          {
            tmp.push_back(':');
            tmp += password;
          }
          c = curl_easy_setopt(curl, CURLOPT_USERPWD, tmp.c_str());
          if(c != CURLE_OK) ret = c;
          return ret;
        }
      private:
        std::string user;
        std::string password;
        std::string caCertFile;
        std::string caCertPath;
        bool insecure;

      };
    } // details

    std::shared_ptr<BasicCurlOpt> Url(const std::string & url,
                                      const std::vector<std::pair<std::string, std::string>> & parameters) {
      return std::make_shared<details::Url>(url, parameters);
    }
    
    std::shared_ptr<BasicCurlOpt> SslPem(const std::string & _CACertFile,
                                         const std::string & _certFile,
                                         bool                _insecure,
                                         const std::string & _passphrase,
                                         const std::string & _caCert,
                                         const std::string & _caCertPath)
    {
      return std::make_shared<details::SslPem>(_CACertFile,
                                               _certFile,
                                               _insecure,
                                               _passphrase,
                                               _caCert,
                                               _caCertPath);
    }

    std::shared_ptr<BasicCurlOpt> HttpAuth(const std::string & _user,
                                           const std::string & _password,
                                           bool                _insecure,
                                           const std::string & _caCert,
                                           const std::string & _caCertPath)
    {
      return std::make_shared<details::HttpAuth>(_user, _password, _insecure, _caCert, _caCertPath);
    }

    std::shared_ptr<BasicCurlOpt> Port(long port) {
      return std::make_shared<details::CurlOpt<long, CURLOPT_PORT>>(port);
    }

    std::shared_ptr<BasicCurlOpt> Delete() {
      return std::make_shared<details::CurlOpt<std::string, CURLOPT_CUSTOMREQUEST>>("DELETE");
    }

    std::shared_ptr<BasicCurlOpt> Data(const std::string & data) {
      return std::make_shared<details::DataBuffer>(data);
    }

    std::shared_ptr<BasicCurlOpt> Header(const std::initializer_list<std::string> & _headers) {
      return std::make_shared<details::HeaderList>(_headers);
    }
    
    std::shared_ptr<BasicCurlOpt> Header(const std::vector<std::string> & _headers) {
      return std::make_shared<details::HeaderList>(_headers);
    }

    std::shared_ptr<BasicCurlOpt> Verbose(bool verbose) {
      return std::make_shared<details::CurlOpt<long, CURLOPT_VERBOSE>>(verbose ? 1L : 0L);
    }

    std::shared_ptr<BasicCurlOpt> Session(CURLSH * share)
    {
      if(share)
      {
        return std::make_shared<details::CurlOpt<CURLSH*, CURLOPT_SHARE>>(share);
      }
      else
      {
        return std::shared_ptr<details::CurlOpt<CURLSH*, CURLOPT_SHARE>>();
      }
    }

    static std::shared_ptr<BasicCurlOpt> CacheSessionId(bool do_cache)
    {
      long value = (do_cache ? 1L : 0L);
      return std::make_shared<details::CurlOpt<long, CURLOPT_SSL_SESSIONID_CACHE>>(value);
    }
  }
}

