/*
Copyright 2018-2019, SURFsara
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
#include <surfsara/handle_permissions.h>

using Permissions = surfsara::handle::Permissions;


TEST_CASE("no_handle_permission", "[Permission]" )
{

  Permissions perm(std::vector<std::string>{}, std::vector<std::string>{});
  REQUIRE_FALSE(perm.checkAny());
  REQUIRE_FALSE(perm.checkSome());
  REQUIRE_FALSE(perm.checkUser("user"));
}

TEST_CASE("all_users_handle_permission", "[Permission]" )
{
  Permissions perm(std::vector<std::string>{"*", "testuser"},
                   std::vector<std::string>{"notmygroup"});
  REQUIRE(perm.checkAny());
  REQUIRE(perm.checkSome());
  REQUIRE(perm.checkUser("user"));
  REQUIRE_FALSE(perm.checkGroup("group"));
}

TEST_CASE("all_groups_handle_permission", "[Permission]" )
{

  Permissions perm(std::vector<std::string>{"testuser"},
                   std::vector<std::string>{"notmygroup", "*"});
  REQUIRE(perm.checkAny());
  REQUIRE(perm.checkSome());
  REQUIRE_FALSE(perm.checkUser("user"));
  REQUIRE(perm.checkGroup("group"));
}

TEST_CASE("no_matching_user_or_group_handle_permission", "[Permission]" )
{
  Permissions perm(std::vector<std::string>{"user1", "user2"},
                   std::vector<std::string>{"group1", "group2"});
  REQUIRE_FALSE(perm.checkAny());
  REQUIRE(perm.checkSome());
  REQUIRE_FALSE(perm.checkUser("user"));
  REQUIRE_FALSE(perm.checkGroup("group"));
  REQUIRE(perm.checkUser("user1"));
  REQUIRE(perm.checkGroup("group2"));
  REQUIRE_FALSE(perm.checkGroup("group3"));
}
