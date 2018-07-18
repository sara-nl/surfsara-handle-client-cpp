#pragma once
#include <vector>
#include <string>
#include <utility>

namespace surfsara
{
  namespace handle
  {
    struct I_ReverseLookupClient
    {
      virtual ~I_ReverseLookupClient() {}
      virtual std::vector<std::string> lookup(const std::vector<std::pair<std::string, std::string>> & query) = 0;
    };
  }
}
