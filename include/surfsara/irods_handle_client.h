#pragma once
#include <set>
#include "i_handle_client.h"
#include "i_reverse_lookup_client.h"
#include <surfsara/handle_result.h>
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
    };

    class IRodsHandleClient
    {
    public:
      IRodsHandleClient(std::shared_ptr<I_HandleClient> _handleClient,
                        std::shared_ptr<I_ReverseLookupClient> _reverseLookupClient,
                        const IRodsConfig & _config) :
        config(_config),
        handleClient(_handleClient),
        reverseLookupClient(_reverseLookupClient) {}

      inline Result create(const std::string & path);
      inline Result update(const std::string & oldPath, const std::string & newPath);
      inline Result remove(const std::string & path);
      inline Result get(const std::string & path);

    private:
      inline std::vector<int> update(IndexAllocator & alloc, surfsara::ast::Node & root, const std::string & path);
      IRodsConfig config;
      std::shared_ptr<I_HandleClient> handleClient;
      std::shared_ptr<I_ReverseLookupClient> reverseLookupClient;
    };
  } // handle
}

namespace surfsara
{
  namespace handle
  {
    inline std::vector<int> IRodsHandleClient::update(IndexAllocator & alloc, surfsara::ast::Node & root, const std::string & path)
    {
      using Node = surfsara::ast::Node;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      using Undefined = surfsara::ast::Undefined;
      std::string url = surfsara::util::joinPath(config.urlPrefix, path);
      std::vector<int> removeIndices;
      updateIndex(alloc, root, "IRODS_SERVER", String(config.server));
      updateIndex(alloc, root, "IRODS_SERVER_PORT", Integer(config.port));
      updateIndex(alloc, root, "IRODS_URL", String(url));
      if(config.webDavPrefix.empty())
      {
        {
          auto tmp = updateIndex(alloc, root, "IRODS_WEBDAV_URL", Undefined());
          if(tmp.isA<Integer>())
          {
            removeIndices.push_back(tmp.as<Integer>());
          }
        }
        {
          auto tmp = updateIndex(alloc, root, "IRODS_WEBDAV_PORT", Undefined());
          if(tmp.isA<Integer>())
          {
            removeIndices.push_back(tmp.as<Integer>());
          }
        }
        updateIndex(alloc, root, "URL", String(url));
        updateIndex(alloc, root, "PORT", Integer(config.port));
      }
      else
      {
        std::string webdavUrl = surfsara::util::joinPath(config.webDavPrefix, path);
        updateIndex(alloc, root, "IRODS_WEBDAV_URL", String(webdavUrl));
        updateIndex(alloc, root, "IRODS_WEBDAV_PORT", Integer(config.webDavPort));
        updateIndex(alloc, root, "URL", String(webdavUrl));
        updateIndex(alloc, root, "PORT", Integer(config.webDavPort));
      }
      return removeIndices;
    }

    inline Result IRodsHandleClient::create(const std::string & path)
    {
      using Object = surfsara::ast::Object;
      using Array = surfsara::ast::Array;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      std::string url = surfsara::util::joinPath(config.urlPrefix, path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS_URL", url}});
      if(lookupResult.empty())
      {
        IndexAllocator alloc;
        Object admin{
          {"index", Integer(100)},
          {"type", String("HS_ADMIN")},
          {"data", Object({
              {"format", String("admin")},
              {"value", Object({
                    {"handle", String(std::string("0.NA/") + config.irodsPrefix)},
                    {"index", Integer(200)},
                    {"permissions", String("011111110011")}})}})}};
        Node root = Object{{"values", Array({admin})}};
        update(alloc, root, path);
        return handleClient->create(config.irodsPrefix, root);
      }
      else
      {
        throw ValidationError({std::string("Object with irods URL already exists: ") + url});
      }
    }

    inline Result IRodsHandleClient::update(const std::string & oldPath, const std::string & newPath)
    {
      std::string url = surfsara::util::joinPath(config.urlPrefix, oldPath);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS_URL", url}});
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
          IndexAllocator alloc(getIndices(obj.data));
          auto removedIndices = update(alloc, obj.data, newPath);
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
      std::string url = surfsara::util::joinPath(config.urlPrefix, path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS_URL", url}});
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
      std::string url = surfsara::util::joinPath(config.urlPrefix, path);
      auto lookupResult = reverseLookupClient->lookup({{"IRODS_URL", url}});
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

  }
}
