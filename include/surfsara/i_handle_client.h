#pragma once
#include <surfsara/handle_result.h>
#include <surfsara/ast.h>

namespace surfsara
{
  namespace handle
  {
    struct I_HandleClient
    {
      virtual ~I_HandleClient() {}
      virtual Result create(const std::string & prefix, const surfsara::ast::Node & node) = 0;
      virtual Result get(const std::string & handle) = 0;
      virtual Result update(const std::string & handle, const surfsara::ast::Node & node) = 0;
      virtual Result removeIndices(const std::string & handle, const std::vector<int> & indices) = 0;
      virtual Result remove(const std::string & handle) = 0;
    };
  }
}
