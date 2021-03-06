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
    class IRodsHandleClient
    {
    public:
      IRodsHandleClient(std::shared_ptr<I_HandleClient> _handleClient,
                        const std::string & _handlePrefix,
                        std::shared_ptr<I_ReverseLookupClient> _reverseLookupClient,
                        std::shared_ptr<HandleProfile> _profile,
                        bool _do_lookup_before,
                        //const IRodsConfig & _config,
                        const std::string & _lookupKey,
                        const std::string & _lookupValue) :
        handleClient(_handleClient),
        handlePrefix(_handlePrefix),
        profile(_profile),
        reverseLookupClient(_reverseLookupClient),
        do_lookup_before(_do_lookup_before),
        lookupKey(_lookupKey),
        lookupValue(_lookupValue)
      {}

      inline Result create(const std::string & paths,
                           const std::vector<std::pair<std::string, std::string>> & kvpairs);

      inline Result moveHandle(const std::string & handle, const std::string & newPath);
      inline Result move(const std::string & oldPath, const std::string & newPath);

      inline Result removeHandle(const std::string & handle);
      inline Result remove(const std::string & path);

      inline Result get(const std::string & path);
      inline Result getHandle(const std::string & handle);

      /**
       * Update a set of indices of a handle
       */
      inline Result setHandle(const std::string & handle,
                              const std::vector<std::pair<std::string, std::string>> & kvpairs);
      inline Result set(const std::string & path,
                        const std::vector<std::pair<std::string, std::string>> & kvpairs);

      inline Result unsetHandle(const std::string & handle,
                                const std::vector<std::string> & keys);
      inline Result unset(const std::string & path,
                          const std::vector<std::string> & keys);


      /**
       * Lookup the handle for the given path.
       * @return vector of matching handles.
       */
      inline std::vector<std::string> lookup(const std::string & path);

      /**
       * Attempts to find exactly one handle.
       * Throws execption if not found
       */
      inline std::string lookupOne(const std::string & path);

    private:
      std::shared_ptr<I_HandleClient> handleClient;
      std::string handlePrefix;
      std::shared_ptr<I_ReverseLookupClient> reverseLookupClient;
      std::shared_ptr<HandleProfile> profile;
      bool do_lookup_before;
      std::string lookupKey;
      std::string lookupValue;
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
      using Object = surfsara::ast::Object;
      using Array = surfsara::ast::Array;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      std::map<std::string, std::string> object_repl_map{{"{OBJECT}", path}};
      if(do_lookup_before)
      {
        auto value = profile->expand(lookupValue, object_repl_map);
        auto lookupResult = reverseLookupClient->lookup({{lookupKey, value}});
        if(!lookupResult.empty())
        {
          throw ValidationError({std::string("Object with ") + lookupKey + "=" + value + " already exists."});
        }
      }
      return handleClient->create(handlePrefix, profile->create(object_repl_map,
                                                                kvp));
    }

    inline Result IRodsHandleClient::moveHandle(const std::string & handle, const std::string & newPath)
    {
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
        throw ValidationError({std::string("Failed to retriev handle / decode ") + handle});
      }
    }

    inline Result IRodsHandleClient::move(const std::string & oldPath, const std::string & newPath)
    {
      return moveHandle(lookupOne(oldPath), newPath);
    }

    inline Result IRodsHandleClient::removeHandle(const std::string & handle)
    {
      return handleClient->remove(handle);
    }

    inline Result IRodsHandleClient::remove(const std::string & path)
    {
      return removeHandle(lookupOne(path));
    }

    inline Result IRodsHandleClient::getHandle(const std::string & handle)
    {
      return handleClient->get(handle);
    }

    inline Result IRodsHandleClient::get(const std::string & path)
    {
      auto value = profile->expand(lookupValue, {{"{OBJECT}", path}});
      auto lookupResult = reverseLookupClient->lookup({{lookupKey, value}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for ") + lookupKey + "=" + value});
      }
      else
      {
        std::string handle(lookupResult[0]);
        return handleClient->get(handle);
      }
    }

    inline Result IRodsHandleClient::setHandle(const std::string & handle,
                                               const std::vector<std::pair<std::string, std::string>> & kvp)
    {
      auto obj = handleClient->get(handle);
      if(obj.success)
      {
        profile->setIndices(obj.data, kvp);
        return handleClient->update(handle, obj.data);
      }
      else
      {
        throw ValidationError({std::string("Failed to retriev handle / decode ") + handle});
      }
    }

    inline Result IRodsHandleClient::set(const std::string & path,
                                         const std::vector<std::pair<std::string, std::string>> & kvp)
    {
      using Integer = surfsara::ast::Integer;
      using Undefined = surfsara::ast::Undefined;
      using String = surfsara::ast::String;
      auto value = profile->expand(lookupValue, {{"{OBJECT}", path}});
      auto lookupResult = reverseLookupClient->lookup({{lookupKey, value}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for ") + lookupKey + "=" + value});
      }
      else
      {
        return setHandle(lookupResult[0], kvp);
      }
    }

    inline Result IRodsHandleClient::unsetHandle(const std::string & handle,
                                                 const std::vector<std::string> & keys)
    {
      auto obj = handleClient->get(handle); 
      if(obj.success)
      {
        std::vector<int> removeIndices = profile->unsetIndices(obj.data, keys);
        return handleClient->removeIndices(handle, removeIndices);
      }
      else
      {
        throw ValidationError({std::string("Failed to retriev handle / decode ") + handle});
      }
    }

    inline Result IRodsHandleClient::unset(const std::string & path,
                                           const std::vector<std::string> & keys)
    {
      using Integer = surfsara::ast::Integer;
      using Undefined = surfsara::ast::Undefined;
      auto value = profile->expand(lookupValue, {{"{OBJECT}", path}});
      auto lookupResult = reverseLookupClient->lookup({{lookupKey, value}});
      if(lookupResult.empty())
      {
        throw ValidationError({std::string("Could not find PID for ") + lookupKey + "=" + value});
      }
      else
      {
        return unsetHandle(lookupResult[0], keys);
      }
    }

    inline std::vector<std::string> IRodsHandleClient::lookup(const std::string & path)
    {
      auto value = profile->expand(lookupValue, {{"{OBJECT}", path}});
      return reverseLookupClient->lookup({{lookupKey, value}});
    }

    inline std::string IRodsHandleClient::lookupOne(const std::string & path)
    {
      auto value = profile->expand(lookupValue, {{"{OBJECT}", path}});
      auto lookupResult = reverseLookupClient->lookup({{lookupKey, value}});
      if(lookupResult.size() == 1)
      {
        return lookupResult[0];
      }
      else if(lookupResult.size() == 0)
      {
        throw ValidationError({std::string("Could not find PID for ") + lookupKey + "=" + value});
      }
      else
      {
        throw ValidationError({std::string("Could not find PID for ") + lookupKey + "=" + value + " not unique, found " + std::to_string(lookupResult.size()) + " matching entries"});
      }
    }
  }
}
