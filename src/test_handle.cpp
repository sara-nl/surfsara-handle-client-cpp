// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <surfsara/handle_validation.h>
#include <surfsara/json_format.h>
#include <surfsara/ast.h>

using Node = surfsara::ast::Node;
using Array = surfsara::ast::Array;
using Object = surfsara::ast::Object;
using Null = surfsara::ast::Null;
using String = surfsara::ast::String;
using Integer = surfsara::ast::Integer;
using namespace surfsara::handle;

TEST_CASE( "validateIndices", "[Handle]" ) {
  Node node = Array{};
  auto idx1 = Object{{"index", 1}};
  auto idx100 = Object{{"index", 100},
                       {"type", "test"},
                       {"data", Object()}};
  REQUIRE_THROWS(getIndices(Null()));
  REQUIRE_THROWS(getIndices(Object()));
  REQUIRE_THROWS(getIndices(Node(Array{})));
  REQUIRE(getIndices(Object{{"values", Array{}}}) == std::vector<int>{});
  REQUIRE_THROWS(getIndices(Object{{"values", Array{Integer(1)}}}));
  REQUIRE_THROWS(getIndices(Object{{"values", Array{Object{}}}}));
  REQUIRE_THROWS(getIndices(Object{{"values", Array{Object{{"index", String("abc")}}}}}));


  REQUIRE_THROWS(getIndices(Object{{"values", Array{idx1, idx1}}}));
  REQUIRE(getIndices(Object{{"values", Array{idx1, idx100}}}) == std::vector<int>{1,100});
}
