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
