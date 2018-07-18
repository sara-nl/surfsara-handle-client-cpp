#include <catch2/catch.hpp>
#include <surfsara/util.h>

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
