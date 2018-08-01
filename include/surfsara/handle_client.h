#pragma once
#include "i_handle_client.h"
#include <surfsara/handle_result.h>
#include <surfsara/handle_util.h>
#include <surfsara/curl.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>
#include <surfsara/util.h>

namespace surfsara
{
  namespace handle
  {
    class HandleClient : public I_HandleClient
    {
    public:
      HandleClient(const std::string & url,
                   std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> options = {},
                   bool _verbose = false);

      Result create(const std::string & prefix, const surfsara::ast::Node & node) override
      {
        return createImpl(prefix, node);
      }

      Result get(const std::string & handle) override
      {
        return getImpl(handle);
      }

      Result move(const std::string & handle, const surfsara::ast::Node & node) override
      {
        return moveImpl(handle, node);
      }

      Result removeIndices(const std::string & handle, const std::vector<int> & indices) override
      {
        return removeIndicesImpl(handle, indices);
      }

      Result remove(const std::string & handle) override
      {
        return removeImpl(handle);
      }

      /* helpers */
      inline std::string generateHandle(const std::string & prefix);
      inline const std::string& getUrl() const;
      inline std::string getUrlWithHandle(const std::string & handle) const;

    private:
      inline Result createImpl(const std::string & prefix, const surfsara::ast::Node & node);
      inline Result getImpl(const std::string & handle);
      inline Result moveImpl(const std::string & handle, const surfsara::ast::Node & node);
      inline Result removeIndicesImpl(const std::string & handle, const std::vector<int> & indices);
      inline Result removeImpl(const std::string & handle);
      inline static void extractResponse(Result & res, const surfsara::ast::Node & json);
      inline Result curlRequest(const std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> & optionsCopy);
      std::string url;
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
                                      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> _options,
                                      bool _verbose)
      : url(_url), options(_options), verbose(_verbose)
    {
    }

    inline Result HandleClient::createImpl(const std::string & prefix, const Node & node)
    {
      using namespace surfsara::ast;
      std::string handle = generateHandle(prefix);
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(handle),
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

    inline Result HandleClient::getImpl(const std::string & handle)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(handle), {}));
      return curlRequest(optionsCopy);
    }

    inline Result HandleClient::moveImpl(const std::string & handle, const surfsara::ast::Node & node)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      std::vector<std::pair<std::string, std::string>> options{{"overwrite", "true"}};
      for(auto idx : getIndices(node))
      {
        options.push_back(std::make_pair("index", std::to_string(idx)));
      }
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(handle), options));
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      optionsCopy.push_back(surfsara::curl::Data(surfsara::ast::formatJson(node)));
      if(verbose)
      {
        std::cout << "request data:" << std::endl
                  << surfsara::ast::formatJson(node, true) << std::endl;
      }
      return curlRequest(optionsCopy);
    }
    
    inline Result HandleClient::removeIndicesImpl(const std::string & handle, const std::vector<int> & indices)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      std::vector<std::pair<std::string, std::string>> params;
      for(auto ind : indices)
      {
        params.push_back(std::make_pair("index", std::to_string(ind)));
      }
      optionsCopy.push_back(surfsara::curl::Delete());
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(handle), params));
      return curlRequest(optionsCopy);

    }

    inline Result HandleClient::removeImpl(const std::string & handle)
    {
      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>> optionsCopy(options);
      optionsCopy.push_back(surfsara::curl::Header({"Content-Type:application/json", "Authorization: Handle clientCert=\"true\""}));
      optionsCopy.push_back(surfsara::curl::Delete());
      optionsCopy.push_back(surfsara::curl::Url(getUrlWithHandle(handle), {}));
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

    std::string HandleClient::generateHandle(const std::string & prefix)
    {
      std::stringstream tmp;
      boost::uuids::random_generator gen;
      tmp << gen();
      return surfsara::util::joinPath(prefix, tmp.str());
    }

    const std::string& HandleClient::getUrl() const
    {
      return url;
    }

    std::string HandleClient::getUrlWithHandle(const std::string & handle) const
    {
      return surfsara::util::joinPath(url, handle);
    }
  }
}
