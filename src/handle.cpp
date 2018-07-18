#include "handle_operation.h"
#include <surfsara/curl.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>
#include <surfsara/ast.h>
#include <surfsara/util.h>

#include <surfsara/handle_config.h>
#include <surfsara/handle_client.h>
#include <surfsara/reverse_lookup_client.h>
#include <surfsara/handle_util.h>
#include <surfsara/irods_handle_client.h>

#include <cli.h>
#include <iostream>
#include <fstream>

using HandleClient = surfsara::handle::HandleClient;
using ValidationError = surfsara::handle::ValidationError;
using Node = surfsara::ast::Node;


////////////////////////////////////////////////////////////////////////////////
//
// Create
//
////////////////////////////////////////////////////////////////////////////////
class HandleCreate : public HandleOperation
{
public:
  HandleCreate() : HandleOperation("create",
                                   "create <JSON>: create a new PID from json\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() != 1)
    {
      std::cerr << "exactly 1 argument (json) expected for operation create" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(HandleProgramArgs & args) override
  {
    auto client = createHandleClient(args);
    surfsara::handle::Result res;
    Node node(surfsara::ast::parseJson(args.args->getValue().front()));
    if(args.verbose->isSet())
    {
      std::cout << args.config.handle_prefix->getValue() << std::endl;
    }
    res = client->create(args.config.handle_prefix->getValue(), node);
    return finalize(args, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Get
//
////////////////////////////////////////////////////////////////////////////////
class HandleGet : public HandleOperation
{
public:
  HandleGet(): HandleOperation("get",
                               "get <HANDLE>: get PID\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (handle) required for get operation" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(HandleProgramArgs & args) override
  {
    auto client = createHandleClient(args);
    surfsara::handle::Result res;
    res = client->get(args.args->getValue().front());
    return finalize(args, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Update
//
////////////////////////////////////////////////////////////////////////////////
class HandleUpdate : public HandleOperation
{
public:
  HandleUpdate(): HandleOperation("update",
                                  "update <HANDLE> <JSON>: update PID (only inidices that appear in the JSON doc)\n"
                                  "update --replace <HANDLE> <JSON>: replace PID\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() != 2)
    {
      std::cerr << "exactly 2 arguments (handle and json) expected for operation create" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(HandleProgramArgs & args) override
  {
    auto client = createHandleClient(args);
    surfsara::handle::Result res;
    Node node(surfsara::ast::parseJson(args.args->getValue()[1]));
    res = client->update(args.args->getValue().front(), node);
    return finalize(args, res);
  }
};


////////////////////////////////////////////////////////////////////////////////
//
// Resolve
//
////////////////////////////////////////////////////////////////////////////////
class HandleResolve : public HandleOperation
{
public:
  HandleResolve():  HandleOperation("resolve",""){}
  virtual int parse(HandleProgramArgs & args) override
  {
    return 0;
  }
  
  virtual int exec(HandleProgramArgs & args) override
  {
    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Lookup
//
////////////////////////////////////////////////////////////////////////////////
class HandleLookup : public HandleOperation
{
public:
  HandleLookup(): HandleOperation("lookup",
                                  "lookup <KEY>=<VALUE> <KEY>=<VALUE> ...: find a handle by reverse lookup\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    int ret = 0;
    if(!checkLookupParameters(args))
    {
      ret = 8;
    }
    return ret;
  }

  virtual int exec(HandleProgramArgs & args) override
  {
    surfsara::handle::ReverseLookupClient client(args.config.lookup_url->getValue(),
                                                 args.config.handle_prefix->getValue(),
                                                 std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                                   surfsara::curl::Verbose(args.verbose->isSet()),
                                                     surfsara::curl::Port(args.config.lookup_port->getValue()),
                                                     surfsara::curl::HttpAuth(args.config.lookup_user->getValue(),
                                                                              args.config.lookup_password->getValue(),
                                                                              args.config.lookup_insecure->isSet())},
                                                 (args.config.lookup_limit->isSet() ? args.config.lookup_limit->getValue() : 100),
                                                 (args.config.lookup_page->isSet() ? args.config.lookup_page->getValue() : 0),
                                                 args.verbose->isSet());
    std::vector<std::pair<std::string, std::string>> query;
    for(auto arg : args.args->getValue())
    {
      std::size_t pos = arg.find('=');
      if(pos != std::string::npos)
      {
        query.push_back(std::make_pair(std::string(arg.begin(), arg.begin() + pos),
                                       std::string(arg.begin() + pos + 1, arg.end())));
      }
    }
    auto res = client.lookup(query);
    for(auto handle : res)
    {
      std::cout << handle << std::endl;
    }
    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Delete
//
////////////////////////////////////////////////////////////////////////////////
class HandleDelete : public HandleOperation
{
public:
  HandleDelete(): HandleOperation("delete",
                                  "delete <HANDLE> <KEY> <KEY>: delete a key from PID\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() == 0)
    {
      std::cerr << "required at least index to be removed" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(HandleProgramArgs & args) override
  {
    std::vector<std::string> values = args.args->getValue();
    std::string handle = values.front();
    auto client = createHandleClient(args);
    values.erase(values.begin());
    std::vector<int> indices;
    for(auto arg : values)
    {
      indices.push_back(surfsara::util::fromString<int>(arg));
    }
    auto res = client->removeIndices(handle, indices);
    return finalize(args, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// DeletePid
//
////////////////////////////////////////////////////////////////////////////////
class HandleDeletePid : public HandleOperation
{
public:
  HandleDeletePid(): HandleOperation("delete_pid",
                                     "delete_pid <HANDLE>: delete full PID") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (handle) required for get operation" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(HandleProgramArgs & args) override
  {
    auto client = createHandleClient(args);
    std::string handle = args.args->getValue().front();
    auto res = client->remove(handle);
    return finalize(args, res);
  }
};


////////////////////////////////////////////////////////////////////////////////
//
// Create IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleCreateIRodsObject : public HandleOperation
{
public:
  HandleCreateIRodsObject() : HandleOperation("icreate",
                                              "icreate <IRODS_PATH>: create a new PID for irods object\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    if(args.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (irods path) required for create operation" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(HandleProgramArgs & args) override
  {
    using ReverseLookupClient = surfsara::handle::ReverseLookupClient;
    using HandleClient = surfsara::handle::HandleClient;
    using IRodsHandleClient = surfsara::handle::IRodsHandleClient;
    auto handleClient = createHandleClient(args);
    auto reverseLookupClient = std::make_shared<ReverseLookupClient>(args.config.lookup_url->getValue(),
                                                                     args.config.handle_prefix->getValue(),
                                                                     std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                                                       surfsara::curl::Verbose(args.verbose->isSet()),
                                                                         surfsara::curl::Port(args.config.lookup_port->getValue()),
                                                                         surfsara::curl::HttpAuth(args.config.lookup_user->getValue(),
                                                                                                  args.config.lookup_password->getValue(),
                                                                                                  args.config.lookup_insecure->isSet())},
                                                                     (args.config.lookup_limit->isSet() ? args.config.lookup_limit->getValue() : 100),
                                                                     (args.config.lookup_page->isSet() ? args.config.lookup_page->getValue() : 0),
                                                                     args.verbose->isSet());
    IRodsHandleClient client(handleClient, reverseLookupClient,
                             surfsara::handle::IRodsConfig(args.config.irods_url_prefix->getValue(),
                                                           args.config.irods_server->getValue(),
                                                           args.config.handle_prefix->getValue(),
                                                           args.config.irods_port->getValue(),
                                                           args.config.irods_webdav_prefix->getValue(),
                                                           args.config.irods_webdav_port->getValue()));

    //client.create(args.args->getValue().front());
    //
    //surfsara::handle::Result res;
    //Node node(surfsara::ast::parseJson());
    //res = client.create(node);
    //return finalize(args, res);
    //HandleClient client(createHandleClient(args, args.config.handle_prefix->getValue()));
    //surfsara::handle::Result res;
    //Node node(surfsara::ast::parseJson(args.args->getValue().front()));
    //res = client.create(node);
    //return finalize(args, res);
    return 8;
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Update IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleUpdateIRodsObject : public HandleOperation
{
public:
  HandleUpdateIRodsObject(): HandleOperation("iupdate",
                                             "iupdate <OLD_PATH> <NEW_PATH>: update PID for irods object\n") {}
  virtual int parse(HandleProgramArgs & args) override
  {
    return 8;
  }

  virtual int exec(HandleProgramArgs & args) override
  {
    return 8;
  }
};

///////////////////////////////////////////////////////////////////////////////
int main(int argc, const char ** argv)
{
  HandleProgramArgs args;
  args.addOperation<HandleCreate>();
  args.addOperation<HandleGet>();
  args.addOperation<HandleUpdate>();
  args.addOperation<HandleResolve>();
  args.addOperation<HandleLookup>();
  args.addOperation<HandleDelete>();
  args.addOperation<HandleDeletePid>();
  args.addOperation<HandleCreateIRodsObject>();
  args.addOperation<HandleUpdateIRodsObject>();
  args.registerArguments();

  auto op = args.parse(argc, argv);
  if(!op)
  {
    if(args.help->isSet())
    {
      return 0;
    }
    else
    {
      return 8;
    }
  }
  auto ret = op->parse(args);
  if(ret != 0)
  {
    args.parser.printHelp(std::cout);
    return ret;
  }
  else
  {
    return op->exec(args);
  }
  return 0;
}
