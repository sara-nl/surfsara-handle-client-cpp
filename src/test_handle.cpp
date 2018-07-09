// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <surfsara/handle_validation.h>
#include <surfsara/json_format.hpp>
#include <surfsara/ast.hpp>

using Node = surfsara::ast::Node;
using Array = surfsara::ast::Array;
using Object = surfsara::ast::Object;
using Null = surfsara::ast::Null;
using namespace surfsara::handle;

void validate(std::function<std::vector<std::string>(const Node & node)> func, const Node & node)
{
  auto res = func(node);
  if(!res.empty())
  {
    throw ValidationError(res);
  }
}

TEST_CASE( "validateIndex", "[Handle]" ) {
  REQUIRE_THROWS(validate(&validateIndex, Null()));
  REQUIRE_THROWS(validate(&validateIndex, Object()));
  REQUIRE_THROWS(validate(&validateIndex, Array()));
  REQUIRE_THROWS(validate(&validateIndex, Array{1,2,3}));
  REQUIRE_THROWS(validate(&validateIndex, Object{{"index", 1}}));
  REQUIRE_THROWS(validate(&validateIndex, Object{{"index", 1}, {"type", "test"}}));
  REQUIRE_THROWS(validate(&validateIndex, Object{{"index", 1},
                                                 {"type", "test"},
                                                 {"data", Null()}}));
  REQUIRE_THROWS(validate(&validateIndex, Object{{"index", "t"},
                                                 {"type", "test"},
                                                 {"data", Object()}}));
  REQUIRE_NOTHROW(validate(&validateIndex, Object{{"index", 1},
                                                  {"type", "test"},
                                                  {"data", Object()}}));
}

TEST_CASE( "validateResource", "[Handle]" ) {
  REQUIRE_THROWS(validate(&validateIndex, Null()));
  REQUIRE_THROWS(validate(&validateResource, 1));
  REQUIRE_THROWS(validate(&validateResource, Array{1,2,3}));
  REQUIRE_NOTHROW(validate(&validateResource,
                           Array{Object({
                                 {"index", 1},
                                 {"type", "test"},
                                 {"data", Object()}})}));
}
