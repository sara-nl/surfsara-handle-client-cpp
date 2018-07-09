#pragma once
#include <stdexcept>
#include <vector>
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
      ValidationError(std::vector<std::string> & errs);
    };

    /**
       Validate:
       [{ "index": {INDEX},
          "type": {TYPE},
          "data": {DATA} },
        ...]
      @returns vector of error messages
    */
    std::vector<std::string> validateResource(const Node & node);
    std::vector<std::string> validateIndex(const Node & node);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// implmentation
//
////////////////////////////////////////////////////////////////////////////////
inline surfsara::handle::ValidationError::ValidationError(std::vector<std::string> & errs)
  : std::invalid_argument(boost::algorithm::join(errs, " "))
{
}

std::vector<std::string> surfsara::handle::validateIndex(const Node & node)
{
  using Object = surfsara::ast::Object;
  using Integer = surfsara::ast::Integer;
  std::vector<std::string> ret;
  std::cout << formatJson(node) << std::endl;
  if(node.isA<Object>())
  {
    std::vector<std::string> essentials{"type", "index", "data"};
    bool ok = true;
    for(auto ess : essentials)
    {
      if(!node.as<Object>().has(ess))
      {
        ret.push_back(std::string("missing key: ") + ess);
        ok = false;
      }
    }
    if(ok)
    {
      if(!node.as<Object>().get("index").isA<Integer>())
      {
        ret.push_back(std::string("index must be an integer."));
      }
      if(!node.as<Object>().get("data").isA<Object>())
      {
        ret.push_back(std::string("data must be an object."));
      }
    }
  }
  else
  {
    ret.push_back(std::string("expected a object, ") +
                  node.typeName() +
                  " given: " +
                  surfsara::ast::formatJson(node));
  }
  return ret;
}

std::vector<std::string> surfsara::handle::validateResource(const Node & node)
{
  using Array = surfsara::ast::Array;
  using Object = surfsara::ast::Object;
  std::vector<std::string> ret;
  if(node.isA<Array>())
  {
    node.as<Array>().forEach([&ret](const Node & n) {
        auto tmp = surfsara::handle::validateIndex(n);
        ret.insert(ret.end(), tmp.begin(), tmp.end());
    });
  }
  else
  {
    ret.push_back(std::string("expected a list, ") + node.typeName() + " given.");
  }
  return ret;
}


