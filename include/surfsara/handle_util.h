#pragma once
#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <boost/algorithm/string/join.hpp>
#include <surfsara/ast.h>
#include <surfsara/json_format.h>

namespace surfsara
{
  namespace handle
  {
    using Node = ::surfsara::ast::Node;

    class ValidationError : public std::invalid_argument
    {
    public:
      ValidationError(const std::vector<std::string> & errs);
    };

    struct IndexAllocator
    {
      IndexAllocator(const std::vector<int> & used = {},
                     int _maxI = 100);
      inline surfsara::ast::Integer operator()();
    private:
      std::set<int> used;
      std::set<int>::const_iterator itr;
      surfsara::ast::Integer next;
      int maxI;
    };


    inline const surfsara::ast::Array & getIndexArray(const surfsara::ast::Node & node);
    inline std::vector<int> getIndices(const surfsara::ast::Node & node);
    inline std::vector<int> getIndices(const surfsara::ast::Array & array);

    inline surfsara::ast::Node getIndexByType(const surfsara::ast::Node & rootNode,
                                              const std::string & type);
    inline surfsara::ast::Node getIndexByType(const surfsara::ast::Array & array,
                                              const std::string & type);

    inline surfsara::ast::Node updateIndex(IndexAllocator & alloc,
                                           surfsara::ast::Node & root,
                                           const std::string & type,
                                           const surfsara::ast::Node & value);

  }
}

////////////////////////////////////////////////////////////////////////////////
//
// implmentation
//
////////////////////////////////////////////////////////////////////////////////
namespace surfsara
{
  namespace handle
  {
    inline ValidationError::ValidationError(const std::vector<std::string> & errs)
      : std::invalid_argument(boost::algorithm::join(errs, " "))
    {
    }

    ////////////////////////////////////////////////////////////////////////////////
    inline IndexAllocator::IndexAllocator(const std::vector<int> & _used, int _maxI)
    {
      for(auto i : _used)
      {
        if(i < _maxI)
        {
          used.insert(i);
        }
      }
      used.insert(_maxI);
      itr = used.begin();
      next = 0;
      maxI = _maxI;
    }

    inline surfsara::ast::Integer IndexAllocator::operator()()
    {
      // |
      // 1,2,3
      while(itr != used.end() && next < maxI)
      {
        ++next;
        if(next != *itr)
        {
          return next;
        }
        ++itr;
      }
      throw std::out_of_range("cannot allocate a new index in the range [1,100)");
    }

    ////////////////////////////////////////////////////////////////////////////////
    inline const surfsara::ast::Array & getIndexArray(const surfsara::ast::Node & node)
    {
      using Object = surfsara::ast::Object;
      using Array = surfsara::ast::Array;
      if(node.isA<Object>())
      {
        if(node.as<Object>().has("values"))
        {
          if(node.as<Object>()["values"].isA<Array>())
          {
            return node.as<Object>()["values"].as<Array>();
          }
          else
          {
            throw ValidationError({
                std::string("invalid type ") +
                node.as<Object>()["values"].typeName() + " expected Array"});
          }
        }
        else
        {
          throw ValidationError({std::string("values not found in top level object")});
        }
      }
      else
      {
        throw ValidationError({std::string("expected object, given ") + node.typeName()});
      }
    }

    inline std::vector<int> getIndices(const surfsara::ast::Node & node)
    {
      return getIndices(getIndexArray(node));
    }

    inline std::vector<int> getIndices(const surfsara::ast::Array & array)
    {
      using Object = surfsara::ast::Object;
      using Integer = surfsara::ast::Integer;
      std::set<int> ret;
      std::vector<std::string> errs;
      array.forEach([&ret, &errs](const Node & node) {
          if(node.isA<Object>())
          {
            if(node.as<Object>().has("index"))
            {
              if(node.as<Object>().get("index").isA<Integer>())
              {
                int i = node.as<Object>().get("index").as<Integer>();
                if(ret.find(i) == ret.end())
                {
                  ret.insert(i);
                }
                else
                {
                  errs.push_back("duplicate index " +
                                 std::to_string(i) + ":" +
                                 surfsara::ast::formatJson(node));
                }
              }
              else
              {
                errs.push_back(std::string("key 'index' must be an integer."));
              }
            }
            else
            {
              errs.push_back(std::string("missing key 'index'"));
            }
          }
          else
          {
            errs.push_back(std::string("expected a object, ") +
                           node.typeName() +
                           " given: " +
                           surfsara::ast::formatJson(node));
          }
        });
      if(!errs.empty())
      {
        throw ValidationError(errs);
      }
      return std::vector<int>(ret.begin(), ret.end());
    }

    inline surfsara::ast::Node getIndexByType(const surfsara::ast::Node & node,
                                              const std::string & type)
    {
      return getIndexByType(getIndexArray(node), type);      
    }
    
    inline surfsara::ast::Node getIndexByType(const surfsara::ast::Array & array,
                                              const std::string & type)
    {
      using Object = surfsara::ast::Object;
      using String = surfsara::ast::String;
      using Node = surfsara::ast::Node;
      using Undefined = surfsara::ast::Undefined;
      std::vector<std::string> errs;
      Node ret = Undefined();
      array.forEach([&ret, &errs, &type](const Node & node) {
          if(node.isA<Object>() &&
             node.as<Object>().has("type") &&
             node.as<Object>().get("type").as<String>() == type)
          {
            ret = node; 
          }
        });
      return ret;
    }

    inline surfsara::ast::Node updateIndex(IndexAllocator & alloc,
                                           surfsara::ast::Node & root,
                                           const std::string & type,
                                           const surfsara::ast::Node & value)
    {
      using Undefined = surfsara::ast::Undefined;
      using Node = surfsara::ast::Node;
      using String = surfsara::ast::String;
      using Object = surfsara::ast::Object;
      using Integer = surfsara::ast::Integer;
      if(value.isA<Undefined>())
      {
        auto n = getIndexByType(root, type);
        root.remove("values/*",
                    [&type](const Node & r,
                            const std::vector<std::string> & path) {
                      std::vector<std::string> tp(path);
                      tp.push_back("type");
                      return (r.find(tp) == String(type));
                    });
        if(n.isA<Object>() && n.as<Object>().has("index") && n.as<Object>()["index"].isA<Integer>())
        {
          return n.as<Object>()["index"];
        }
        else
        {
          return Undefined();
        }
      }
      else
      {
        auto node = getIndexByType(root, type);
        if(node == Undefined())
        {
          root.update("values/#", Node(Object{{"index", alloc()},
                                              {"type", String(type)},
                                              {"value", value}}));
        }
        else
        {
          root.update("values/*/value", value, true, [&type](const Node & root, const std::vector<std::string> & path) {
              std::vector<std::string> tp(path);
              tp.pop_back();
              tp.push_back("type");
              return (root.find(tp) == String(type));
            });
        }
        return Undefined();
      }
    }

  } // handle 
} // surfsara


