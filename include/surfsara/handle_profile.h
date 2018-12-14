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
#include <surfsara/ast.h>
#include <map>
#include <set>
#include "handle_util.h"
#include "util.h"

namespace surfsara
{
  namespace handle
  {
    class HandleProfile
    {
    public:
      HandleProfile(const surfsara::ast::Node & _profile,
                    const std::map<std::string, std::string> & _parameters,
                    long _index_from = 2,
                    long _index_to = 100);
      HandleProfile(const std::map<std::string, std::string> & _parameters);
      inline surfsara::ast::Node create(const std::map<std::string, std::string> & additional_parameters,
                                        const std::vector<std::pair<std::string, std::string>> & kvp);
      inline std::vector<int> update(surfsara::ast::Node & root,
                                     const std::map<std::string, std::string> & additional_parameters);
      inline void setIndices(surfsara::ast::Node & root,
                             const std::vector<std::pair<std::string, std::string>> & kvp);
      inline std::vector<int> unsetIndices(surfsara::ast::Node & root,
                                           const std::vector<std::string> & kvp);
      inline std::string expand(const std::string & templ,
                                const std::map<std::string, std::string> & additional_parameters = {});
      inline std::set<std::string> getKeys() const;
    private:
      static inline surfsara::ast::Object createStringIndex(const std::string & key,
                                                            const std::string & value);
      inline surfsara::ast::Object expandIndexObject(const surfsara::ast::Node & obj) const;
      inline void extractRemoved(const surfsara::ast::Node & obj,
                                 std::set<std::string> & keep,
                                 std::set<std::string> & remove);
      inline std::vector<int> updateRemove(surfsara::ast::Node & root);
      long index_from;
      long index_to;
      surfsara::ast::Node profile;
      std::map<std::string, std::string> parameters;
    };
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
//////////////////////////////////////////////////////////////////////////////////////////
inline surfsara::handle::HandleProfile::HandleProfile(const surfsara::ast::Node & _profile,
                                                      const std::map<std::string, std::string> & _parameters,
                                                      long _index_from,
                                                      long _index_to) :
  index_from(_index_from),
  index_to(_index_to),
  profile(_profile),
  parameters(_parameters)
{
}

inline surfsara::handle::HandleProfile::HandleProfile(const std::map<std::string, std::string> & _parameters)
  : parameters(_parameters)
{
  using Node = surfsara::ast::Node;
  using Array = surfsara::ast::Array;
  using Object = surfsara::ast::Object;
  using Integer = surfsara::ast::Integer;
  using Pair = surfsara::ast::Pair;
  index_from = 2;
  index_to = 100;
  profile = Array{
    Object{
      Pair{"if_set", "IRODS_WEBDAV_PREFIX"},
      Pair{"entry", Object{
          Pair{"index", Integer(1)},
          Pair{"type", "URL"},
          Pair{"data", Object{
              Pair{"format", "string"},
              Pair{"value",  "{IRODS_WEBDAV_PREFIX}{OBJECT}"}}}}}},
    Object{
      Pair{"if_not_set", "IRODS_WEBDAV_PREFIX"},
      Pair{"entry", Object{
          Pair{"index", Integer(1)},
          Pair{"type", "URL"},
          Pair{"data", Object{
              Pair{"format", "string"},
              Pair{"value", "{IRODS_URL_PREFIX}{OBJECT}"}}}}}},
    HandleProfile::createStringIndex("IRODS/URL", "{IRODS_URL_PREFIX}{OBJECT}"),
    Object{
      Pair{"if_set", "IRODS_WEBDAV_PREFIX"},
      Pair{"entry", Object{
          Pair{"index", "{INDEX}"},
          Pair{"type", "IRODS/WEBDAV_URL"},
          Pair{"data", Object{
              Pair{"format", "string"},
              Pair{"value",  "{IRODS_WEBDAV_PREFIX}{OBJECT}"}}}}}},
    HandleProfile::createStringIndex("IRODS/SERVER", "{IRODS_SERVER}"),
    HandleProfile::createStringIndex("IRODS/SERVER_PORT", "{IRODS_PORT}"),
    Object{
      Pair{"entry", Object{
          Pair{"index", 100},
          Pair{"type", "HS_ADMIN"},
          Pair{"data", Object{
              Pair{"format", "admin"},
              Pair{"value", Object{
                  Pair{"handle", "0.NA/{HANDLE_PREFIX}"},
                  Pair{"index", 200},
                  Pair{"permissions", "011111110011"}}}}}}}}};
}

inline surfsara::ast::Object surfsara::handle::HandleProfile::createStringIndex(const std::string & key,
                                                                                       const std::string & value)
{
  using Object = surfsara::ast::Object;
  using Pair = surfsara::ast::Pair;
  return Object{
    Pair{"entry", Object{
        Pair{"index", "{INDEX}"},
          Pair{"type", key},
          Pair{"data", Object{
              Pair{"format", "string"},
              Pair{"value", value}}}}}};
}

inline surfsara::ast::Object surfsara::handle::HandleProfile::expandIndexObject(const surfsara::ast::Node & n) const
{
  using Undefined = surfsara::ast::Undefined;
  using Object = surfsara::ast::Object;
  using String = surfsara::ast::String;
  if(n.isA<Object>())
  {
    const Object & obj(n.as<Object>());
    const Node & entry(obj.get("entry"));
    if(entry.isA<Object>())
    {
      auto if_set = obj.get("if_set");
      if(if_set.isA<String>())
      {
        if(parameters.find(if_set.as<String>()) != parameters.end())
        {
          return entry.as<Object>();
        }
        else
        {
          return Object{};
        }
      }
      auto if_not_set = obj.get("if_not_set");
      if(if_not_set.isA<String>())
      {
        if(parameters.find(if_not_set.as<String>()) == parameters.end())
        {
          return entry.as<Object>();
        }
        else
        {
          return Object{};
        }
      }
      return entry.as<Object>();
    }
  }
  return Object{};
}

inline void surfsara::handle::HandleProfile::extractRemoved(const surfsara::ast::Node & n,
                                                            std::set<std::string> & keep,
                                                            std::set<std::string> & remove)
{
  using Undefined = surfsara::ast::Undefined;
  using Object = surfsara::ast::Object;
  using String = surfsara::ast::String;
  if(n.isA<Object>())
  {
    const Object & obj(n.as<Object>());
    const Node & entry(obj.get("entry"));
    if(entry.isA<Object>())
    {
      auto t = entry.as<Object>().get("type");
      if(!t.isA<String>())
      {
        return;
      }
      auto if_set = obj.get("if_set");
      bool always_set = true;
      if(if_set.isA<String>())
      {
        always_set = false;
        if(parameters.find(if_set.as<String>()) != parameters.end())
        {
          keep.insert(t.as<String>());
        }
        else
        {
          remove.insert(t.as<String>());
        }
      }
      auto if_not_set = obj.get("if_not_set");
      if(if_not_set.isA<String>())
      {
        always_set = false;
        if(parameters.find(if_not_set.as<String>()) == parameters.end())
        {
          keep.insert(t.as<String>());
        }
        else
        {
          remove.insert(t.as<String>());
        }
      }
      if(always_set)
      {
        keep.insert(t.as<String>());
      }
    }
  }
}

inline surfsara::ast::Node surfsara::handle::HandleProfile::create(const std::map<std::string, std::string> & additional_parameters,
                                                                   const std::vector<std::pair<std::string, std::string>> & kvp)
{
  using Array = surfsara::ast::Array;
  using Node = surfsara::ast::Node;
  using Object = surfsara::ast::Object;
  using Pair = surfsara::ast::Pair;
  using String = surfsara::ast::String;
  using Undefined = surfsara::ast::Undefined;
  Array result;
  std::map<std::string, std::string> params(additional_parameters);
  for(auto kv : parameters)
  {
    params[std::string("{") + kv.first + "}"] = kv.second;
  }
  if(profile.isA<Array>())
  {
    profile.as<Array>().forEach([this, &result](const Node & n){
        auto obj = expandIndexObject(n);
        if(!obj.empty())
        {
          result.pushBack(obj);
        }
      });
  }
  else
  {
    throw std::logic_error(std::string("Profile is not an array: ") + ast::formatJson(profile, true));
  }
  std::set<std::string> keys = getKeys();
  auto root = Node(Object{Pair{"values", result}});
  for(auto & kv : kvp)
  {
    if(keys.find(kv.first) == keys.end())
    {
        updateIndex(root, kv.first, String(kv.second));
    }
    else
    {
      // duplicates are not allowed
      std::string msg("indices defined in the template cannot be overwritten: ");
      msg += kv.first;
      throw std::logic_error(msg);
    }
  }
  IndexAllocator alloc(getIndices(profile.as<Array>(), false),
                       index_from,
                       index_to);
  deepReplace(root, params, alloc);
  return root;
}


inline std::vector<int> surfsara::handle::HandleProfile::updateRemove(surfsara::ast::Node & root)
{
  using Array = surfsara::ast::Array;
  using Node = surfsara::ast::Node;
  using Integer = surfsara::ast::Integer;
  using Undefined = surfsara::ast::Undefined;

  std::set<std::string> keep;
  std::set<std::string> remove;
  std::vector<int> ret;
  profile.as<Array>().forEach([this, &keep, &remove](const Node & n){
      extractRemoved(n, keep, remove);
    });
  for(auto r : remove)
  {
    if(keep.find(r) == keep.end())
    {
      Node n = updateIndex(root, r, Undefined());
      if(n.isA<Integer>())
      {
        ret.push_back(n.as<Integer>());
      }
    }
  }
  return ret;
}
                                                                

inline std::vector<int> surfsara::handle::HandleProfile::update(surfsara::ast::Node & root,
                                                                const std::map<std::string, std::string> & additional_parameters)
{
  using Array = surfsara::ast::Array;
  using Object = surfsara::ast::Object;
  using String = surfsara::ast::String;
  using Integer = surfsara::ast::Integer;
  using Undefined = surfsara::ast::Undefined;
  profile.as<Array>().forEach([this, &root](const Node & n){
      Object obj = surfsara::handle::HandleProfile::expandIndexObject(n);
      if(!obj.empty())
      {
        if(obj.has("type"))
        {
          auto t = obj.get("type");
          if(t.isA<String>())
          {
            if(t.as<String>() != "HS_ADMIN")
            {
              updateIndex(root, t.as<String>(), n.find("entry/data/value"), obj.get("index"));
            }
          }
        }
      }
  });
  IndexAllocator alloc(getIndices(root.as<Object>().get("values").as<Array>(), false),
                       index_from,
                       index_to);
  std::map<std::string, std::string> params(additional_parameters);
  for(auto kv : parameters)
  {
    params[std::string("{") + kv.first + "}"] = kv.second;
  }
  deepReplace(root, params, alloc);
  return updateRemove(root);
}

inline void surfsara::handle::HandleProfile::setIndices(surfsara::ast::Node & root,
                                                        const std::vector<std::pair<std::string, std::string>> & kvp)
{
  using String = surfsara::ast::String;
  for(auto & kv : kvp)
  {
    updateIndex(root, kv.first, String(kv.second));
  }
  IndexAllocator alloc(getIndices(root, false), index_from, index_to);
  std::map<std::string, std::string> params;
  for(auto kv : parameters)
  {
    params[std::string("{") + kv.first + "}"] = kv.second;
  }
  deepReplace(root, params, alloc);
}

inline std::vector<int> surfsara::handle::HandleProfile::unsetIndices(surfsara::ast::Node & root,
                                                                      const std::vector<std::string> & kvp)
{
  using Integer = surfsara::ast::Integer;
  using Undefined = surfsara::ast::Undefined;
  std::vector<int> removedIndices;
  for(auto & key : kvp)
  {
    auto tmp = updateIndex(root, key, Undefined());
    if(tmp.isA<Integer>())
    {
      removedIndices.push_back(tmp.as<Integer>());
    }
  }
  return removedIndices;
}

inline std::string surfsara::handle::HandleProfile::expand(const std::string & templ,
                                                           const std::map<std::string, std::string> & additional_parameters)
{
  std::string ret(templ);
  for(const auto & kp : parameters)
  {
    surfsara::util::replace(ret,
                            std::string("{") + kp.first + std::string("}"),
                            kp.second);
  }
  for(const auto & kp : additional_parameters)
  {
    surfsara::util::replace(ret, kp.first, kp.second);
  }
  return ret;
}

inline std::set<std::string> surfsara::handle::HandleProfile::getKeys() const
{
  using Array = surfsara::ast::Array;
  using String = surfsara::ast::String;
  using Object = surfsara::ast::Object;
  std::set<std::string> keys;
  profile.forEach("*/entry/type", [&keys](const Node & root,
                                     const std::vector<std::string> & path) {
                    auto n = root.find(path);
                    if(n.isA<String>())
                    {
                      keys.insert(n.as<String>());
                    }
                  });
  return keys;
}


    
