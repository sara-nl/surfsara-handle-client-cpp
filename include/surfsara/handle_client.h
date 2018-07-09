#pragma once
#include <surfsara/handle_operation.h>
#include <surfsara/curl.h>
#include <surfsara/json_format.h>

namespace surfsara
{
  namespace handle
  {
    using Node = ::surfsara::ast::Node;

    class HandleClient
    {
    public:
      HandleClient(const std::string & url,
                   std::initializer_list<std::shared_ptr<surfsara::curl::details::BasicCurlOpt>> options);
      Node create(const Node & node, const std::string & uuid = "");
      Node update(const std::string & uuid, const std::vector<Operation> & operations);
      Node remove(const std::string & uuid);
    private:
      std::string url;
      std::vector<std::shared_ptr<surfsara::curl::details::BasicCurlOpt>> options;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////////////
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

inline
surfsara::handle::HandleClient::HandleClient(const std::string & _url,
                                             std::initializer_list<std::shared_ptr<surfsara::curl::details::BasicCurlOpt>> _options)
  : url(_url), options(_options)
{
}

surfsara::ast::Node surfsara::handle::HandleClient::create(const Node & node,
                                                           const std::string & uuid)
{
  std::stringstream surl;
  surl << url;
  if(url.back() != '/')
  {
    surl << url << "/";
  }
  if(uuid.empty())
  {
    boost::uuids::random_generator gen;
    surl << gen();
  }
  else
  {
    surl << uuid;
  }
  //std::vector<std::shared_ptr<surfsara::curl::details::BasicCurlOpt>> optCopy(options);
  //optCopy.push_back(surfsara::curl::Url(surl.str()));
  //optCopy.push_back(surfsara::curl::Data(surfsara::ast::formatJson(node)));
  //surfsara::curl::Curl curl(optCopy);
  //auto res = curl.request();
  return node;
}

surfsara::ast::Node surfsara::handle::HandleClient::update(const std::string & uuid,
                                                           const std::vector<Operation> & operations)
{
  return surfsara::ast::Node();
}

surfsara::ast::Node surfsara::handle::HandleClient::remove(const std::string & uuid)
{
  return surfsara::ast::Node();
}

//////////////////////////////////
// adding handle and assigns uuid
//
// ./handle create url/prefix '[{ index: 1, type: type, data: {"abc": 1}}]'
//
//////////////////////////////////
// get handle
//
// ./handle get url/prefix/suffix                            # all -> list
// ./handle get url/prefix/suffix?index=1&index=2&type=XX    # filtered objects -> list
// ./handle get url/prefix/suffix/{INDEX}                    # specific index -> object
// ./handle get url/prefix/suffix/{INDEX}/type               # type for specific index -> string
// ./handle get url/prefix/suffix/{INDEX}/data               # data object for specific index -> object
// ./handle get url/prefix/suffix/{INDEX}/data/field         # field of data object -> atom or object
// ./handle get url/prefix/suffix/{INDEX}/data/pa/th         # field of suboject -> atom or object
//
//////////////////////////////////
// update
//
// replace all indices
// ./handle put url/prefix/suffix/ '[{ index: 1, type: type, data: {"abc": 1}}]'
//
// type and data for one index
// ./handle put url/prefix/suffix/{INDEX} '{"type": {TYPE}", "data": {...}}'
//
// only type
// ./handle put url/prefix/suffix/{INDEX}/type {TYPE}
//
// only data
// ./handle put url/prefix/suffix/{INDEX}/data '{"abc": 1}'
//
// specific field
// ./handle put url/prefix/suffix/{INDEX}/data/path/to/field 42
#if 0              
Handle handle("http://handle/prefix", option);

std::string json =
  R"___(
[{"index": 1, "type": "url", "data": { "abc": 1, "def": 2}},
 {"index": 10, "type": "url", "data": { "abc": 1, "def": 2}}]
)___";

/* iRods
handleCreate(url, json) -> handleid, success, msg
 */
handle.create(json);


handle.update("{suffix}",
              { Update("1/data/url", Node("http://dffdff")),
                Delete("100/data/this/remove_this_key"),
                Update("1/data/retries", Node(1)) });

/* iRods
handleUpdate(list(url, list(list("U", "1/data/url", '"http://dffdff"'),
             list("D", "100/data/this/remove_this_key"),
             list("U", "1/dat/retries", 1)))) --> success, msg
*/
handle.update("{suffix}",
              { Update("1/data/url", Node("http://dffdff")),
                Delete("100/data/this/remove_this_key"),
                Update("1/data/retries", Node(1)) });


/*
handleRemove(url)
*/
handle.remove("{suffix}");

      
#endif
