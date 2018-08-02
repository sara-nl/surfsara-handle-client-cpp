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
