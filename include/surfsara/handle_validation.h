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

    inline std::vector<int> getIndices(const surfsara::ast::Node & node);
    inline std::vector<int> getIndices(const surfsara::ast::Array & array);
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

    std::vector<int> getIndices(const surfsara::ast::Array & array)
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

    std::vector<int> getIndices(const surfsara::ast::Node & node)
    {
      using Object = surfsara::ast::Object;
      using Array = surfsara::ast::Array;
      std::vector<int> ret;
      std::vector<std::string> errs;
      if(node.isA<Object>())
      {
        if(node.as<Object>().has("values"))
        {
          if(node.as<Object>()["values"].isA<Array>())
          {
            return getIndices(node.as<Object>()["values"].as<Array>());
          }
          else
          {
            errs.push_back(std::string("invalid type ") +
                           node.as<Object>()["values"].typeName() +
                           " expected Array");
          }
        }
        else
        {
          errs.push_back(std::string("values not found in top level object"));
        }
      }
      else
      {
        errs.push_back(std::string("expected object, given ") + node.typeName());
      }
      if(!errs.empty())
      {
        throw ValidationError(errs);
      }
      return ret;
    }
  } // handle 
} // surfsara


