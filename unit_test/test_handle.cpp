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
#include <surfsara/handle_util.h>
#include <surfsara/irods_handle_client.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>
#include <surfsara/ast.h>

using Node = surfsara::ast::Node;
using Array = surfsara::ast::Array;
using Object = surfsara::ast::Object;
using Null = surfsara::ast::Null;
using String = surfsara::ast::String;
using Integer = surfsara::ast::Integer;
using Undefined = surfsara::ast::Undefined;

using namespace surfsara::handle;

////////////////////////////////////////////////////////////////////////////////
//
// Mocks
//
////////////////////////////////////////////////////////////////////////////////

struct HandleClientMock : public I_HandleClient
{
  std::function<Result(const std::string & prefix, const surfsara::ast::Node & node)> mockCreate;
  std::function<Result(const std::string & handle)> mockGet;
  std::function<Result(const std::string & handle, const surfsara::ast::Node & node)> mockUpdate;
  std::function<Result(const std::string & handle, const std::vector<int> & indices)> mockRemoveIndices;
  std::function<Result(const std::string & handle)> mockRemove;

  
  virtual Result create(const std::string & handle, const surfsara::ast::Node & node) override
  {
    return mockCreate(handle, node);
  }

  virtual Result get(const std::string & handle) override
  {
    return mockGet(handle);
  }

  virtual Result update(const std::string & handle, const surfsara::ast::Node & node) override
  {
    return mockUpdate(handle, node);
  }

  virtual Result removeIndices(const std::string & handle, const std::vector<int> & indices) override
  {
    return mockRemoveIndices(handle, indices);
  }

  virtual Result remove(const std::string & handle) override
  {
    return mockRemove(handle);
  }
};

struct ReverseLookupClientMock : public I_ReverseLookupClient
{
  std::function<std::vector<std::string>(const std::vector<std::pair<std::string, std::string>>&)> mockLookup;

  virtual std::vector<std::string> lookup(const std::vector<std::pair<std::string, std::string>> & query)
  {
    return mockLookup(query);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// helper functions
//
////////////////////////////////////////////////////////////////////////////////

TEST_CASE( "validateIndices", "[Handle]" )
{
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

TEST_CASE("attempt to create index without free void throws", "[IndexAllocator]" )
{
  IndexAllocator alloc({1,2,3}, 1, 4);
  REQUIRE_THROWS(alloc());
}

TEST_CASE("create indices", "[IndexAllocator]" )
{
  {
    IndexAllocator alloc({2,4,6}, 1, 8);
    REQUIRE(alloc() == 1);
    REQUIRE(alloc() == 3);
    REQUIRE(alloc() == 5);
    REQUIRE(alloc() == 7);
    REQUIRE_THROWS(alloc());
  }
  {
    IndexAllocator alloc({}, 2, 8);
    REQUIRE(alloc() == 2);
    REQUIRE(alloc() == 3);
    REQUIRE(alloc() == 4);
    REQUIRE(alloc() == 5);
    REQUIRE(alloc() == 6);
    REQUIRE(alloc() == 7);
    REQUIRE_THROWS(alloc());
  }
  {
    IndexAllocator alloc({1,2,4}, 2, 8);
    REQUIRE(alloc() == 3);
    REQUIRE(alloc() == 5);
    REQUIRE(alloc() == 6);
    REQUIRE(alloc() == 7);
    REQUIRE_THROWS(alloc());
  }
}


TEST_CASE("update", "[Handle]" )
{
  IndexAllocator alloc({2,4});
  Node root = Object{{"values", Array({})}};
  updateIndex(root, "TEST_INDEX", String("value"));
  deepReplace(root, {}, alloc);
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":3,\"type\":\"TEST_INDEX\",\"data\":{\"format\":\"string\",\"value\":\"value\"}}]}");
  updateIndex(root, "TEST_INDEX", String("value2"));
  deepReplace(root, {}, alloc);
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":3,\"type\":\"TEST_INDEX\",\"data\":{\"format\":\"string\",\"value\":\"value2\"}}]}");
  updateIndex(root, "TEST_INDEX2", String("value"));
  updateIndex(root, "URL", String("url-value"));
  deepReplace(root, {}, alloc);
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":3,\"type\":\"TEST_INDEX\",\"data\":{\"format\":\"string\",\"value\":\"value2\"}},"
          "{\"index\":5,\"type\":\"TEST_INDEX2\",\"data\":{\"format\":\"string\",\"value\":\"value\"}},"
          "{\"index\":6,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"url-value\"}}]}");
  updateIndex(root, "TEST_INDEX", Undefined());
  deepReplace(root, {}, alloc);
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":5,\"type\":\"TEST_INDEX2\",\"data\":{\"format\":\"string\",\"value\":\"value\"}},"
          "{\"index\":6,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"url-value\"}}]}");
}

TEST_CASE("deep update", "[Handle]")
{
  using String = surfsara::ast::String;
  using Object = surfsara::ast::Object;
  using Node = surfsara::ast::Node;
  using Pair = surfsara::ast::Pair;
  using Integer = surfsara::ast::Integer;
  Node n = Object{
    Pair{"entry", Object{
        Pair{"index", "{INDEX}"},
          Pair{"type", "URL"},
          Pair{"data", Object{
              Pair{"format", "string"},
              Pair{"value",  "webdav://{IRODS_WEBDAV_PREFIX}:{IRODS_WEBDAV_PORT}{OBJECT}"}}}}}};

  IndexAllocator alloc({}, 42, 100);
  deepReplace(n,
              {{"{IRODS_WEBDAV_PREFIX}", "PREFIX"},
                {"{IRODS_WEBDAV_PORT}", "PORT"},
                {"{OBJECT}", "/obj"}},
              alloc);


  REQUIRE(n.find("entry/data/value").as<String>() == "webdav://PREFIX:PORT/obj");
  REQUIRE(n.find("entry/index").as<Integer>() == 42);
}

////////////////////////////////////////////////////////////////////////////////
//
// IRodsHandleClient
//
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("create irods handle", "[IRodsHandleClient]" )
{

  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver:80"},
                               {"IRODS_SERVER", "myserver"},
                               {"HANDLE_PREFIX", "HANDLE"},
                               {"IRODS_PORT", "1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");
  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      REQUIRE(prefix == "prefix");
      Array arr = node.as<Object>()["values"].as<Array>();
      REQUIRE(arr.size() == 8);
      REQUIRE(surfsara::ast::formatJson(arr[0])==
              "{\"index\":1,\"type\":\"URL\","
              "\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[1])==
              "{\"index\":2,\"type\":\"IRODS/URL\","
              "\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[2])==
              "{\"index\":3,\"type\":\"IRODS/WEBDAV_URL\","
              "\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[3])==
              "{\"index\":4,\"type\":\"IRODS/SERVER\","
              "\"data\":{\"format\":\"string\",\"value\":\"myserver\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[4])==
              "{\"index\":5,\"type\":\"IRODS/SERVER_PORT\","
              "\"data\":{\"format\":\"string\",\"value\":\"1247\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[6])==
              "{\"index\":6,\"type\":\"KEY1\","
              "\"data\":{\"format\":\"string\",\"value\":\"VALUE1\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[7])==
              "{\"index\":7,\"type\":\"KEY2\","
              "\"data\":{\"format\":\"string\",\"value\":\"VALUE2\"}}");
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };

  auto res = client.create("/path/to/object.txt",
                           {{"KEY1", "VALUE1"},
                            {"KEY2", "VALUE2"}});
}

TEST_CASE("create irods handle without webdav", "[IRodsHandleClient]" )
{

  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"},
                               {"IRODS_SERVER", "myserver"},
                               {"IRODS_PORT", "1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");

  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      Array arr = node.as<Object>()["values"].as<Array>();
      REQUIRE(arr.size() == 6);
      REQUIRE(surfsara::ast::formatJson(arr[0])==
              "{\"index\":1,\"type\":\"URL\","
              "\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[1])==
              "{\"index\":2,\"type\":\"IRODS/URL\","
              "\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[2])==
              "{\"index\":3,\"type\":\"IRODS/SERVER\","
              "\"data\":{\"format\":\"string\",\"value\":\"myserver\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[3])==
              "{\"index\":4,\"type\":\"IRODS/SERVER_PORT\","
              "\"data\":{\"format\":\"string\",\"value\":\"1247\"}}");
      // arr4: HS_ADMIN
      REQUIRE(surfsara::ast::formatJson(arr[5])==
              "{\"index\":5,\"type\":\"DUMMY\","
              "\"data\":{\"format\":\"string\",\"value\":\"VALUE\"}}");
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };
  auto res = client.create("/path/to/object.txt", {{"DUMMY", "VALUE"}});
}

TEST_CASE("overwriting default fields throws", "[IRodsHandleClient]")
{
  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver:80"},
                               {"IRODS_SERVER", "myserver"},
                               {"HANDLE_PREFIX", "HANDLE"},
                               {"IRODS_PORT", "1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");

  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };
  REQUIRE_THROWS(client.create("/path/to/object.txt",
                               {{"URL", "VALUE"},
                                 {"IRODS/URL", "VALUE_OVWRT"}}));
}

TEST_CASE("create duplicate irods handle throws", "[IRodsHandleClient]" )
{
  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");
  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      // must not be called
      REQUIRE_FALSE(true);
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"dummy-prefix/dummy-suffix"});
    };
  REQUIRE_THROWS(client.create("/path/to/object.txt", {}));
}

TEST_CASE("update undefined irods handle throws", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver/"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver/"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };
  REQUIRE_THROWS(client.move("/path/to/object.txt", "/new/path/to/object.txt"));
}

TEST_CASE("update irods handle with webdav", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver:80"},
                               {"IRODS_SERVER", "myserver"},
                               {"IRODS_PORT", "1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix-uuid"});
    };

  handleClient->mockGet = [](const std::string & handle)
    {
      Result res;
      res.success = true;
      res.data = surfsara::ast::parseJson("{\"values\":["
                                          "{\"index\":1,\"type\":\"IRODS/SERVER\",\"data\":{\"format\":\"string\",\"value\":\"myserver\"}},"
                                          "{\"index\":2,\"type\":\"IRODS/SERVER_PORT\",\"data\":{\"format\":\"string\",\"value\":\"1247\"}},"
                                          "{\"index\":3,\"type\":\"IRODS/URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/path/to/object.txt\"}},"
                                          "{\"index\":4,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/path/to/object.txt\"}}]}");
      return res;
    };

  bool updated = false;
  bool removed = false;
  handleClient->mockUpdate = [&updated](const std::string & handle, const surfsara::ast::Node & node)
    {
      updated = true;
      REQUIRE(handle == "prefix-uuid");
      Array arr = node.as<Object>()["values"].as<Array>();
      REQUIRE(arr.size() == 5);
      REQUIRE(surfsara::ast::formatJson(arr[0])=="{\"index\":1,\"type\":\"IRODS/SERVER\",\"data\":{\"format\":\"string\",\"value\":\"myserver\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[1])=="{\"index\":2,\"type\":\"IRODS/SERVER_PORT\",\"data\":{\"format\":\"string\",\"value\":\"1247\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[2])=="{\"index\":3,\"type\":\"IRODS/URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/new/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[3])=="{\"index\":4,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/new/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[4])=="{\"index\":5,\"type\":\"IRODS/WEBDAV_URL\",\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/new/path/to/object.txt\"}}");
      Result res;
      return res;
    };

  handleClient->mockRemoveIndices = [&removed](const std::string & handle, const std::vector<int> & indices)
    {
      removed = true;
      Result res;
      return res;
    };

  client.move("/path/to/object.txt", "/new/path/to/object.txt");
  REQUIRE(updated);
  REQUIRE_FALSE(removed);
}

TEST_CASE("update irods handle with webdav removal", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"},
                               {"IRODS_SERVER", "myserver"},
                               {"IRODS_PORT", "1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");

  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix-uuid"});
    };

  handleClient->mockGet = [](const std::string & handle)
    {
      Result res;
      res.success = true;
      res.data = surfsara::ast::parseJson("{\"values\":["
                                          "{\"index\":1,\"type\":\"IRODS/SERVER\",\"data\":{\"format\":\"string\",\"value\":\"myserver\"}},"
                                          "{\"index\":2,\"type\":\"IRODS/SERVER_PORT\",\"data\":{\"format\":\"string\",\"value\":1247}},"
                                          "{\"index\":3,\"type\":\"IRODS/URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver/new/path/to/object.txt\"}},"
                                          "{\"index\":4,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/new/path/to/object.txt\"}},"
                                          "{\"index\":5,\"type\":\"IRODS/WEBDAV_URL\",\"data\":{\"format\":\"string\",\"value\":\"webdav://myserver:80/new/path/to/object.txt\"}},"
                                          "{\"index\":6,\"type\":\"CUSTOM\",\"data\":{\"format\":\"string\",\"value\":\"value\"}}]}");
      return res;
    };

  bool updated = false;
  bool removed = false;
  handleClient->mockUpdate = [&updated](const std::string & handle, const surfsara::ast::Node & node)
    {
      updated = true;
      REQUIRE(handle == "prefix-uuid");
      Array arr = node.as<Object>()["values"].as<Array>();
      REQUIRE(arr.size() == 5);
      REQUIRE(surfsara::ast::formatJson(arr[0])=="{\"index\":1,\"type\":\"IRODS/SERVER\",\"data\":{\"format\":\"string\",\"value\":\"myserver\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[1])=="{\"index\":2,\"type\":\"IRODS/SERVER_PORT\",\"data\":{\"format\":\"string\",\"value\":\"1247\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[2])=="{\"index\":3,\"type\":\"IRODS/URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/new/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[3])=="{\"index\":4,\"type\":\"URL\",\"data\":{\"format\":\"string\",\"value\":\"irods://myserver:1247/new/path/to/object.txt\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[4])=="{\"index\":6,\"type\":\"CUSTOM\",\"data\":{\"format\":\"string\",\"value\":\"value\"}}");
      Result res;
      res.success = true;
      return res;
    };

  handleClient->mockRemoveIndices = [&removed](const std::string & handle, const std::vector<int> & indices)
    {
      REQUIRE(indices == std::vector<int>{5});
      removed = true;
      Result res;
      res.success = true;
      return res;
    };
  client.move("/path/to/object.txt", "/new/path/to/object.txt");
  REQUIRE(updated);
  REQUIRE(removed);
}


TEST_CASE("remove irods handle", "[IRodsHandleClient]" )
{
  //std::cout << surfsara::ast::formatJson(node, true) << std::endl;
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver:1247"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");
  bool removed = false;
  handleClient->mockRemove = [&removed](const std::string & handle)
    {
      REQUIRE(handle == "prefix/uuid");
      removed = true;
      Result res;
      res.success = true;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix/uuid"});
    };
  client.remove("/path/to/object.txt");
  REQUIRE(removed);
}

TEST_CASE("update irods handle metadata", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           "prefix",
                           reverseLookup,
                           std::make_shared<HandleProfile>(std::map<std::string, std::string>{
                               {"IRODS_URL_PREFIX", "irods://myserver/"},
                               {"IRODS_WEBDAV_PREFIX", "webdav://myserver"}}),
                           true,
                           "IRODS/URL",
                           "{IRODS_WEBDAV_PREFIX}{OBJECT}");

  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix-uuid"});
    };

  handleClient->mockGet = [](const std::string & handle)
    {
      Result res;
      res.success = true;
      res.data = surfsara::ast::parseJson("{\"values\":["
                                          "{\"index\":1,\"type\":\"IRODS/SERVER\","
                                          "\"data\":{\"format\":\"string\",\"value\":\"myserver\"}},"
                                          "{\"index\":2,\"type\":\"IRODS/SERVER_PORT\","
                                          "\"data\":{\"format\":\"string\",\"value\":1247}},"
                                          "{\"index\":3,\"type\":\"IRODS/URL\","
                                          "\"data\":{\"format\":\"string\",\"value\":\"irods://myserver/path/to/object.txt\"}},"
                                          "{\"index\":4,\"type\":\"URL\","
                                          "\"data\":{\"format\":\"string\",\"value\":\"irods://myserver/path/to/object.txt\"}},"
                                          "{\"index\":5,\"type\":\"PORT\","
                                          "\"data\":{\"format\":\"string\",\"value\":1247}},"
                                          "{\"index\":6,\"type\":\"OLD_VALUE\","
                                          "\"data\":{\"format\":\"string\",\"value\":\"old\"}}]}");
      return res;
    };

  bool updated = false;
  bool removed = false;
  handleClient->mockUpdate = [&updated](const std::string & handle,
                                        const surfsara::ast::Node & node)
    {
      updated = true;
      REQUIRE(handle == "prefix-uuid");
      Array arr = node.as<Object>()["values"].as<Array>();
      REQUIRE(arr.size() == 8);
      REQUIRE(surfsara::ast::formatJson(arr[5])==
              "{\"index\":6,\"type\":\"OLD_VALUE\","
              "\"data\":{\"format\":\"string\",\"value\":\"new\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[6])==
              "{\"index\":7,\"type\":\"ADDED_VALUE\","
              "\"data\":{\"format\":\"string\",\"value\":\"add1\"}}");
      REQUIRE(surfsara::ast::formatJson(arr[7])==
              "{\"index\":8,\"type\":\"ADDED_VALUE2\","
              "\"data\":{\"format\":\"string\",\"value\":\"add2\"}}");
      Result res;
      return res;
    };

  handleClient->mockRemoveIndices = [&removed](const std::string & handle, const std::vector<int> & indices)
    {
      removed = true;
      Result res;
      return res;
    };

  client.set("/path/to/object.txt", 
             std::vector<std::pair<std::string, std::string>>{
               {"OLD_VALUE", "new"},
               {"ADDED_VALUE", "add1"},
               {"ADDED_VALUE2", "add2"}});
  REQUIRE(updated);
  REQUIRE_FALSE(removed);
}


