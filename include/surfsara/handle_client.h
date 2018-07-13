#pragma once
#include <surfsara/handle_result.h>
#include <surfsara/handle_validation.h>
#include <surfsara/curl.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>

namespace surfsara
{
  namespace handle
  {
    class HandleClient
    {
    public:
      HandleClient(const std::string & url,
                   const std::string & uuid,
                   std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> options = {},
                   bool _verbose = false);

      inline Result create(const surfsara::ast::Node & node);
      inline Result get();
      inline Result update(const surfsara::ast::Node & node);
      inline Result removeIndices(const std::vector<int> & indices);
      inline Result remove();

      /* helpers */
      inline void generateSuffixIfRequired();
      inline const std::string& getUrl() const;
      inline std::string getUrlWithHandle() const;
      inline const std::string& getHandle() const;

    private:
      //inline Result deleteIndices(const std::vector<Operation> & operations);
      inline static void extractResponse(Result & res, const surfsara::ast::Node & json);
      inline Result curlRequest(const std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> & optionsCopy);
      
      std::string url;
      std::string handle;
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> options;
      bool verbose;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////////////
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

namespace surfsara
{
  namespace handle
  {
    inline HandleClient::HandleClient(const std::string & _url,
                                      const std::string & _handle,
                                      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> _options,
                                      bool _verbose)
      : url(_url), handle(_handle), options(_options), verbose(_verbose)
    {
    }

    inline Result HandleClient::create(const Node & node)
    {
      using namespace surfsara::ast;
      generateSuffixIfRequired();
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(),
                                                {{"overwrite","false"}}));
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));


      if(verbose)
      {
        std::cout << "request data:" << std::endl
                  << surfsara::ast::formatJson(node, true) << std::endl;
      }
      optionsCopy.push_back(surfsara::curl::Data(surfsara::ast::formatJson(node)));
      return curlRequest(optionsCopy);
    }

    inline Result HandleClient::get()
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(), {}));
      return curlRequest(optionsCopy);
    }

    inline Result HandleClient::update(const surfsara::ast::Node & node)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      std::vector<std::pair<std::string, std::string>> options{{"overwrite", "true"}};
      for(auto idx : getIndices(node))
      {
        options.push_back(std::make_pair("index", std::to_string(idx)));
      }
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(), options));
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      optionsCopy.push_back(surfsara::curl::Data(surfsara::ast::formatJson(node)));
      if(verbose)
      {
        std::cout << "request data:" << std::endl
                  << surfsara::ast::formatJson(node, true) << std::endl;
      }
      return curlRequest(optionsCopy);
    }
    
    inline Result HandleClient::removeIndices(const std::vector<int> & indices)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      std::vector<std::pair<std::string, std::string>> params;
      for(auto ind : indices)
      {
        params.push_back(std::make_pair("index", std::to_string(ind)));
      }
      optionsCopy.push_back(surfsara::curl::Delete());
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(), params));
      return curlRequest(optionsCopy);

    }

    inline Result HandleClient::remove()
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      optionsCopy.push_back(surfsara::curl::Delete());
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(), {}));
      return curlRequest(optionsCopy);
    }
  }
}

namespace surfsara
{
  namespace handle
  {
    inline void HandleClient::extractResponse(Result & res, const surfsara::ast::Node & json)
    {
      using namespace surfsara::ast;
      if(json.isA<Object>())
      {
        if(json.as<Object>().has("responseCode") &&
           json.as<Object>().get("responseCode").isA<Integer>())
        {
          res.handleCode = json.as<Object>().get("responseCode").as<Integer>();
        }
        if(json.as<Object>().has("handle") &&
           json.as<Object>().get("handle").isA<String>())
        {
          res.handle = json.as<Object>().get("handle").as<String>();
        }
      }
    }

    inline Result HandleClient::curlRequest(const std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> & optionsCopy)
    {
      using namespace surfsara::ast;
      Result res;
      surfsara::curl::Curl curl(optionsCopy);
      res.curlResult = curl.request();
      try
      {
        res.data = surfsara::ast::parseJson(res.curlResult.body);
        extractResponse(res, res.data);
      }
      catch(...)
      {
        res.jsonDecodeError = true;
      }
      res.success = (res.handleCode == 1) && res.curlResult.success && !res.jsonDecodeError;
      return res;
    }

    void HandleClient::generateSuffixIfRequired()
    {
      std::size_t pos = handle.find('/');
      if(pos == std::string::npos)
      {
        std::stringstream tmp;
        boost::uuids::random_generator gen;
        // no suffix defined
        tmp << "/" << gen();
        handle += tmp.str();
      }
    }

    const std::string& HandleClient::getHandle() const
    {
      return handle;
    }

    const std::string& HandleClient::getUrl() const
    {
      return url;
    }

    std::string HandleClient::getUrlWithHandle() const
    {
      return url + "/" + handle;
    }
  }
}
