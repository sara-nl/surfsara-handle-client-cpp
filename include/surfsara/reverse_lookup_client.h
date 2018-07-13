#pragma once
//#include <surfsara/handle_result.h>
//#include <surfsara/handle_validation.h>
#include <surfsara/curl.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>

namespace surfsara
{
  namespace handle
  {
    class ReverseLookupClient
    {
    public:
      ReverseLookupClient(const std::string & url,
                          const std::string & prefix,
                          std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> options,
                          std::size_t _lookup_limit,
                          std::size_t _lookup_page,
                          bool _verbose = false);

      inline std::vector<std::string> lookup(const std::vector<std::pair<std::string, std::string>> & query);
    private:
      std::string url;
      std::string prefix;
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> options;
      std::size_t lookup_limit;
      std::size_t lookup_page;
      bool verbose;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////////////
namespace surfsara
{
  namespace handle
  {
    inline ReverseLookupClient::ReverseLookupClient(const std::string & _url,
                                                    const std::string & _prefix,
                                                    std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> _options,
                                                    std::size_t _lookup_limit,
                                                    std::size_t _lookup_page,
                                                    bool _verbose)
      : url(_url), prefix(_prefix), options(_options),
        lookup_limit(_lookup_limit),
        lookup_page(_lookup_page),
        verbose(_verbose)
    {
    }

    inline std::vector<std::string> ReverseLookupClient::lookup(const std::vector<std::pair<std::string, std::string>> & _query)
    {
      using Array = surfsara::ast::Array;
      using String = surfsara::ast::String;
      std::vector<std::pair<std::string, std::string>> query(_query);
      query.push_back(std::make_pair("limit", std::to_string(lookup_limit)));
      query.push_back(std::make_pair("page", std::to_string(lookup_page)));
      std::vector<std::string> ret;
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Url(url + "/" + prefix, query));
      surfsara::curl::Curl curl(optionsCopy);
      auto res = curl.request();
      if(verbose)
      {
        std::cout << res << std::endl;
      }
      if(!surfsara::curl::httpCodeIsSuccess(res.httpCode))
      {
        std::stringstream tmp;
        tmp << res;
        throw std::logic_error(tmp.str());
      }

      auto node = surfsara::ast::parseJson(res.body);
      if(node.isA<Array>())
      {
        node.as<Array>().forEach([&ret](const surfsara::ast::Node & node){
            if(node.isA<String>())
            {
              ret.push_back(node.as<String>());
            }
            else
            {
              throw std::logic_error("element is not a String");
            }
        });
      }
      else
      {
        throw std::logic_error("did not return an array");
      }
      return ret;
    }
  }
}
