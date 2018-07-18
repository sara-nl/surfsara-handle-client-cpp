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
  IndexAllocator alloc({1,2,3}, 4);
  REQUIRE_THROWS(alloc());
}

TEST_CASE("create indices", "[IndexAllocator]" )
{
  IndexAllocator alloc({2,4,6}, 8);
  REQUIRE(alloc() == 1);
  REQUIRE(alloc() == 3);
  REQUIRE(alloc() == 5);
  REQUIRE(alloc() == 7);
  REQUIRE_THROWS(alloc());
}


TEST_CASE("update", "[Handle]" )
{
  IndexAllocator alloc({2,4});
  Node root = Object{{"values", Array({})}};
  updateIndex(alloc, root, "TEST_INDEX", String("value"));
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":1,\"type\":\"TEST_INDEX\",\"value\":\"value\"}]}");
  updateIndex(alloc, root, "TEST_INDEX", String("value2"));
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":1,\"type\":\"TEST_INDEX\",\"value\":\"value2\"}]}");
  updateIndex(alloc, root, "TEST_INDEX2", String("value"));
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":1,\"type\":\"TEST_INDEX\",\"value\":\"value2\"},"
          "{\"index\":3,\"type\":\"TEST_INDEX2\",\"value\":\"value\"}]}");
  updateIndex(alloc, root, "TEST_INDEX", Undefined());
  REQUIRE(surfsara::ast::formatJson(root) ==
          "{\"values\":["
          "{\"index\":3,\"type\":\"TEST_INDEX2\",\"value\":\"value\"}]}");
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
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "webdav://myserver",
                                       80));
  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      REQUIRE(prefix == "prefix");
      REQUIRE(surfsara::ast::formatJson(node) == 
              "{\"values\":["
              "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
              "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
              "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/path/to/object.txt\"},"
              "{\"index\":4,\"type\":\"IRODS_WEBDAV_URL\",\"value\":\"webdav://myserver/path/to/object.txt\"},"
              "{\"index\":5,\"type\":\"IRODS_WEBDAV_PORT\",\"value\":80},"
              "{\"index\":6,\"type\":\"URL\",\"value\":\"webdav://myserver/path/to/object.txt\"},"
              "{\"index\":7,\"type\":\"PORT\",\"value\":80}]}");
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };

  auto res = client.create("/path/to/object.txt");
}

TEST_CASE("create irods handle without webdav", "[IRodsHandleClient]" )
{

  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "",
                                       80));
  handleClient->mockCreate = [](const std::string & prefix, const surfsara::ast::Node & node)
    {
      REQUIRE(surfsara::ast::formatJson(node) ==
              "{\"values\":["
              "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
              "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
              "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/path/to/object.txt\"},"
              "{\"index\":4,\"type\":\"URL\",\"value\":\"irods://myserver/path/to/object.txt\"},"
              "{\"index\":5,\"type\":\"PORT\",\"value\":1247}]}");
      Result res;
      return res;
    };
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };

  auto res = client.create("/path/to/object.txt");
}

TEST_CASE("create duplicate irods handle throws", "[IRodsHandleClient]" )
{
  auto handleClient = std::make_shared<HandleClientMock>();
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "webdav://myserver",
                                       80));
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
  REQUIRE_THROWS(client.create("/path/to/object.txt"));
}

TEST_CASE("update undefined irods handle throws", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "webdav://myserver",
                                       80));
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>();
    };
  REQUIRE_THROWS(client.update("/path/to/object.txt", "/new/path/to/object.txt"));
}

TEST_CASE("update irods handle with webdav", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "webdav://myserver",
                                       80));
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix-uuid"});
    };

  handleClient->mockGet = [](const std::string & handle)
    {
      Result res;
      res.success = true;
      res.data = surfsara::ast::parseJson("{\"values\":["
                                          "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
                                          "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
                                          "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/path/to/object.txt\"},"
                                          "{\"index\":4,\"type\":\"URL\",\"value\":\"irods://myserver/path/to/object.txt\"},"
                                          "{\"index\":5,\"type\":\"PORT\",\"value\":1247}]}");
      return res;
    };

  bool updated = false;
  bool removed = false;
  handleClient->mockUpdate = [&updated](const std::string & handle, const surfsara::ast::Node & node)
    {
      updated = true;
      REQUIRE(handle == "prefix-uuid");
      REQUIRE(surfsara::ast::formatJson(node) ==
              "{\"values\":["
              "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
              "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
              "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/new/path/to/object.txt\"},"
              "{\"index\":4,\"type\":\"URL\",\"value\":\"webdav://myserver/new/path/to/object.txt\"},"
              "{\"index\":5,\"type\":\"PORT\",\"value\":80},"
              "{\"index\":6,\"type\":\"IRODS_WEBDAV_URL\",\"value\":\"webdav://myserver/new/path/to/object.txt\"},"
              "{\"index\":7,\"type\":\"IRODS_WEBDAV_PORT\",\"value\":80}]}");
      Result res;
      return res;
    };

  handleClient->mockRemoveIndices = [&removed](const std::string & handle, const std::vector<int> & indices)
    {
      removed = true;
      Result res;
      return res;
    };

  client.update("/path/to/object.txt", "/new/path/to/object.txt");
  REQUIRE(updated);
  REQUIRE_FALSE(removed);
}

TEST_CASE("update irods handle with webdav removal", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "",
                                       80));
  reverseLookup->mockLookup = [](const std::vector<std::pair<std::string, std::string>> & query)
    {
      return std::vector<std::string>({"prefix-uuid"});
    };

  handleClient->mockGet = [](const std::string & handle)
    {
      Result res;
      res.success = true;
      res.data = surfsara::ast::parseJson("{\"values\":["
                                          "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
                                          "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
                                          "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/new/path/to/object.txt\"},"
                                          "{\"index\":4,\"type\":\"URL\",\"value\":\"webdav://myserver/new/path/to/object.txt\"},"
                                          "{\"index\":5,\"type\":\"PORT\",\"value\":80},"
                                          "{\"index\":6,\"type\":\"IRODS_WEBDAV_URL\",\"value\":\"webdav://myserver/new/path/to/object.txt\"},"
                                          "{\"index\":7,\"type\":\"IRODS_WEBDAV_PORT\",\"value\":80}]}");
      return res;
    };

  bool updated = false;
  bool removed = false;
  handleClient->mockUpdate = [&updated](const std::string & handle, const surfsara::ast::Node & node)
    {
      updated = true;
      REQUIRE(handle == "prefix-uuid");
      REQUIRE(surfsara::ast::formatJson(node) ==
              "{\"values\":["
              "{\"index\":1,\"type\":\"IRODS_SERVER\",\"value\":\"myserver\"},"
              "{\"index\":2,\"type\":\"IRODS_SERVER_PORT\",\"value\":1247},"
              "{\"index\":3,\"type\":\"IRODS_URL\",\"value\":\"irods://myserver/new/path/to/object.txt\"},"
              "{\"index\":4,\"type\":\"URL\",\"value\":\"irods://myserver/new/path/to/object.txt\"},"
              "{\"index\":5,\"type\":\"PORT\",\"value\":1247}]}");
      Result res;
      return res;
    };

  handleClient->mockRemoveIndices = [&removed](const std::string & handle, const std::vector<int> & indices)
    {
      REQUIRE(indices == std::vector<int>{6, 7});
      removed = true;
      Result res;
      res.success = true;
      return res;
    };
  client.update("/path/to/object.txt", "/new/path/to/object.txt");
  REQUIRE(updated);
  REQUIRE(removed);
}


TEST_CASE("remove irods handle", "[IRodsHandleClient]" )
{
  auto reverseLookup = std::make_shared<ReverseLookupClientMock>();
  auto handleClient = std::make_shared<HandleClientMock>();
  IRodsHandleClient client(handleClient,
                           reverseLookup,
                           IRodsConfig("irods://myserver/",
                                       "myserver",
                                       "prefix",
                                       1247,
                                       "",
                                       80));
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

