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
#include <set>
#include "i_handle_client.h"
#include "i_reverse_lookup_client.h"
#include <surfsara/handle_result.h>
#include <surfsara/handle_profile.h>
#include <surfsara/ast.h>
#include <surfsara/util.h>

namespace surfsara
{
  namespace handle
  {
    struct IRodsConfig
    {
      std::string urlPrefix;
      std::string server;
      std::string irodsPrefix;
      long port;
      std::string webDavPrefix;
      long webDavPort;

      IRodsConfig() : port(1247), webDavPort(80) {}
      IRodsConfig(const IRodsConfig & rhs) :
        urlPrefix(rhs.urlPrefix),
        server(rhs.server),
        irodsPrefix(rhs.irodsPrefix),
        port(rhs.port),
        webDavPrefix(rhs.webDavPrefix),
        webDavPort(rhs.webDavPort) {}
      IRodsConfig(const std::string & _urlPrefix,
                  const std::string & _server,
                  const std::string & _irodsPrefix,
                  long _port,
                  const std::string & _webDavPrefix,
                  long _webDavPort) :
        urlPrefix(_urlPrefix),
        server(_server),
        irodsPrefix(_irodsPrefix),
        port(_port),
        webDavPrefix(_webDavPrefix),
        webDavPort(_webDavPort) {}

      std::string getUrl(const std::string & path) const
      {
        std::string ret(urlPrefix);
        surfsara::util::replace(ret, "{PORT}", std::to_string(port));
        return surfsara::util::joinPath(ret, path);
      }

      std::string getWebDavUrl(const std::string & path) const
      {
        std::string ret(webDavPrefix);
        surfsara::util::replace(ret, "{PORT}", std::to_string(webDavPort));
        return surfsara::util::joinPath(ret, path);
      }
    };

    class IRodsHandleClient
    {
    public:
      IRodsHandleClient(std::shared_ptr<I_HandleClient> _handleClient,
                        std::shared_ptr<I_ReverseLookupClient> _reverseLookupClient,
                        std::shared_ptr<HandleProfile> _profile,
                        const IRodsConfig & _config) :
        config(_config),
        handleClient(_handleClient),
        profile(_profile),
        reverseLookupClient(_reverseLookupClient) {}

      inline Result create(const std::string & paths,
                           const std::vector<std::pair<std::string, std::string>> & kvpairs);
      inline Result move(const std::string & oldPath, const std::string & newPath);
      inline Result remove(const std::string & path);
      inline Result get(const std::string & path);

      /**
       * Update a set of indices of a handle
       */
      inline Result set(const std::string & path,
                        const std::vector<std::pair<std::string, std::string>> & kvpairs);

      inline Result unset(const std::string & path,
                          const std::vector<std::string> & keys);

      inline std::string lookup(const std::string & path);

    private:
      // @todo: remove
      IRodsConfig config;
      std::shared_ptr<I_HandleClient> handleClient;
      std::shared_ptr<I_ReverseLookupClient> reverseLookupClient;
      std::shared_ptr<HandleProfile> profile;
    };
  } // handle
}

namespace surfsara
{
  namespace handle
  {
    inline Result IRodsHandleClient::create(const std::string & path,
                                            const std::vector<std::pair<std::string, std::string>> & kvp)
    {
      //std::cout << surfsara::ast::formatJson(root, true) << std::endl;
      using Object = surfsara::ast::Object;
      using Array = surfsara::ast::Array;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      std::string url = config.getUrl(path);
      /* @todo remove reverse lookup
         https://trello.com/c/sHyiwTpd/27-pid-remove-reverse-lookup-from-create-to-improve-performance
      */
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        return handleClient->create(config.irodsPrefix, profile->create({{"{OBJECT}", path}}, kvp));
      }
      else
      {
        throw ValidationError({std::string("Object with irods URL already exists: ") + url});
      }
    }

    inline Result IRodsHandleClient::move(const std::string & oldPath, const std::string & newPath)
    {
      std::string url = config.getUrl(oldPath);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for iRODS url ") + url});
      }
      else
      {
        std::string handle(lookupResult[0]);
        auto obj = handleClient->get(handle);
        if(obj.success)
        {
          auto removedIndices = profile->update(obj.data, {{"{OBJECT}", newPath}});
          if(!removedIndices.empty())
          {
            auto res = handleClient->removeIndices(handle, removedIndices);
            if(!res.success)
            {
              throw ValidationError({std::string("Failed to remove unused keys")});
            }
          }
          return handleClient->update(handle, obj.data);
        }
        else
        {
          throw ValidationError({std::string("Failed to retriev handle / decode ") + lookupResult[0]});
        }
      }
    }

    inline Result IRodsHandleClient::remove(const std::string & path)
    {
      std::string url = config.getUrl(path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for iRODS url ") + url});
      }
      else
      {
        std::string handle(lookupResult[0]);
        return handleClient->remove(handle);
      }
    }

    inline Result IRodsHandleClient::get(const std::string & path)
    {
      std::string url = config.getUrl(path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for iRODS url ") + url});
      }
      else
      {
        std::string handle(lookupResult[0]);
        return handleClient->get(handle);
      }
    }

    inline Result IRodsHandleClient::set(const std::string & path,
                                         const std::vector<std::pair<std::string, std::string>> & kvp)
    {
      using Integer = surfsara::ast::Integer;
      using Undefined = surfsara::ast::Undefined;
      using String = surfsara::ast::String;
      std::string url = config.getUrl(path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for iRODS url ") + url});
      }
      else
      {
        std::string handle(lookupResult[0]);
        auto obj = handleClient->get(handle);
        if(obj.success)
        {
          profile->setIndices(obj.data, kvp);
          return handleClient->update(handle, obj.data);
        }
        else
        {
          throw ValidationError({std::string("Failed to retriev handle / decode ") + lookupResult[0]});
        }
      }
    }

    inline Result IRodsHandleClient::unset(const std::string & path,
                                           const std::vector<std::string> & keys)
    {
      using Integer = surfsara::ast::Integer;
      using Undefined = surfsara::ast::Undefined;
      std::string url = config.getUrl(path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for iRODS url ") + url});
      }
      else
      {
        std::string handle(lookupResult[0]);
        auto obj = handleClient->get(handle);
        if(obj.success)
        {
          std::vector<int> removeIndices = profile->unsetIndices(obj.data, keys);
          return handleClient->removeIndices(handle, removeIndices);
        }
        else
        {
          throw ValidationError({std::string("Failed to retriev handle / decode ") + lookupResult[0]});
        }
      }
    }

    inline std::string IRodsHandleClient::lookup(const std::string & path)
    {
      std::string url = config.getUrl(path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS/URL", url}});
      if(lookupResult.size() == 0)
      {
        return std::string("");
      }
      else if(lookupResult.size() == 1)
      {
        return lookupResult[0];
      }
      else
      {
        throw ValidationError({std::string("PID for iRods url ") + url + "not unique, found " + std::to_string(lookupResult.size()) + " matching entries"});
      }
    }
  }
}
