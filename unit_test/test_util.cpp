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
#include <catch2/catch.hpp>
#include <surfsara/util.h>
#include <surfsara/ast.h>

using namespace surfsara::util;

TEST_CASE( "joinPath", "[util]" )
{
  REQUIRE(joinPath("abc", "def") == "abc/def");
  REQUIRE(joinPath("/abc", "def") == "/abc/def");
  REQUIRE(joinPath("/abc/def", "ghi") == "/abc/def/ghi");
  REQUIRE(joinPath("/abc/def", "/ghi") == "/abc/def/ghi");
  REQUIRE(joinPath("/abc/def/", "ghi") == "/abc/def/ghi");
  REQUIRE(joinPath("/abc/def/", "/ghi") == "/abc/def/ghi");
  REQUIRE(joinPath("/abc/def///", "///ghi") == "/abc/def/ghi");
  REQUIRE(joinPath("", "ghi") == "/ghi");
  REQUIRE(joinPath("", "/ghi") == "/ghi");
  REQUIRE(joinPath("//", "///ghi") == "/ghi");
  REQUIRE(joinPath("///", "///ghi") == "/ghi");
  REQUIRE(joinPath("/abc/def///", "") == "/abc/def/");
  REQUIRE(joinPath("/abc/def///", "/") == "/abc/def/");
  REQUIRE(joinPath("/abc/def///", "//") == "/abc/def/");
}

TEST_CASE( "replace", "[util]" )
{
  {
    std::string input("abc def");
    replace(input, "{PORT}", "_");
    REQUIRE(input == "abc def");
  }
  {
    std::string input("{PORT} abc def");
    replace(input, "{PORT}", "_");
    REQUIRE(input == "_ abc def");
  }
  {
    std::string input("abc{PORT}def");
    replace(input, "{PORT}", "_");
    REQUIRE(input == "abc_def");
  }
  {
    std::string input("abc def {PORT}");
    replace(input, "{PORT}", "_");
    REQUIRE(input == "abc def _");
  }
  {
    //@todo replace all
    std::string input("abc{PORT}def{PORT}");
    replace(input, "{PORT}", "_");
    REQUIRE(input == "abc_def{PORT}");
  }
}

