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
using Config = surfsara::handle::Config;
using Operation = surfsara::handle::Operation;
using ValidationError = surfsara::handle::ValidationError;
using Node = surfsara::ast::Node;

inline int finalize(const Config & config, const surfsara::handle::Result & res);
inline bool checkLookupParameters(const Config & config);

////////////////////////////////////////////////////////////////////////////////
//
// Create
//
////////////////////////////////////////////////////////////////////////////////
class HandleCreate : public Operation
{
public:
  HandleCreate() : Operation("create",
                             "create <JSON>: create a new PID from json\n") {}

  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 1)
    {
      std::cerr << "exactly 1 argument (json) expected for operation create" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(Config & config) override
  {
    auto client = config.makeHandleClient();
    surfsara::handle::Result res;
    Node node(surfsara::ast::parseJson(config.args->getValue().front()));
    if(config.verbose->isSet())
    {
      std::cout << config.handle_prefix->getValue() << std::endl;
    }
    res = client->create(config.handle_prefix->getValue(), node);
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Get
//
////////////////////////////////////////////////////////////////////////////////
class HandleGet : public Operation
{
public:
  HandleGet(): Operation("get",
                         "get <HANDLE>: get PID\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (handle) required for get operation" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeHandleClient();
    surfsara::handle::Result res;
    res = client->get(config.args->getValue().front());
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Move
//
////////////////////////////////////////////////////////////////////////////////
class HandleMove : public Operation
{
public:
  HandleMove(): Operation("move",
                            "move <HANDLE> <JSON>: move PID (only inidices that appear in the JSON doc)\n"
                            "move --replace <HANDLE> <JSON>: replace PID\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 2)
    {
      std::cerr << "exactly 2 arguments (handle and json) expected for operation create" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(Config & config) override
  {
    auto client = config.makeHandleClient();
    surfsara::handle::Result res;
    Node node(surfsara::ast::parseJson(config.args->getValue()[1]));
    res = client->update(config.args->getValue().front(), node);
    return finalize(config, res);
  }
};


////////////////////////////////////////////////////////////////////////////////
//
// Resolve
//
////////////////////////////////////////////////////////////////////////////////
class HandleResolve : public Operation
{
public:
  HandleResolve():  Operation("resolve",""){}
  virtual int parse(Config & config) override
  {
    return 0;
  }
  
  virtual int exec(Config & config) override
  {
    return 0;
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Lookup
//
////////////////////////////////////////////////////////////////////////////////
class HandleLookup : public Operation
{
public:
  HandleLookup(): Operation("lookup",
                            "lookup <KEY>=<VALUE> <KEY>=<VALUE> ...: find a handle by reverse lookup\n") {}
  virtual int parse(Config & config) override
  {
    int ret = 0;
    if(!checkLookupParameters(config))
    {
      ret = 8;
    }
    return ret;
  }

  virtual int exec(Config & config) override
  {
    auto reverseLookupClient = config.makeReverseLookupClient();
    std::vector<std::pair<std::string, std::string>> query;
    for(auto arg : config.args->getValue())
    {
      std::size_t pos = arg.find('=');
      if(pos != std::string::npos)
      {
        query.push_back(std::make_pair(std::string(arg.begin(), arg.begin() + pos),
                                       std::string(arg.begin() + pos + 1, arg.end())));
      }
    }
    try
    {
      auto res = reverseLookupClient->lookup(query);
      for(auto handle : res)
      {
        std::cout << handle << std::endl;
      }
      return 0;
    }
    catch(const std::exception & ex)
    {
      std::cerr << ex.what() << std::endl;
      return 8;
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Delete
//
////////////////////////////////////////////////////////////////////////////////
class HandleDelete : public Operation
{
public:
  HandleDelete(): Operation("delete",
                                  "delete <HANDLE> <KEY> <KEY>: delete a key from PID\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() == 0)
    {
      std::cerr << "required at least index to be removed" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    std::vector<std::string> values = config.args->getValue();
    std::string handle = values.front();
    auto client = config.makeHandleClient();
    values.erase(values.begin());
    std::vector<int> indices;
    for(auto arg : values)
    {
      indices.push_back(surfsara::util::fromString<int>(arg));
    }
    auto res = client->removeIndices(handle, indices);
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// DeletePid
//
////////////////////////////////////////////////////////////////////////////////
class HandleDeletePid : public Operation
{
public:
  HandleDeletePid(): Operation("delete_pid",
                               "delete_pid <HANDLE>: delete full PID") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (handle) required for get operation" << std::endl;
      return 8;
    }
    return 0;
  }
  
  virtual int exec(Config & config) override
  {
    auto client = config.makeHandleClient();
    std::string handle = config.args->getValue().front();
    auto res = client->remove(handle);
    return finalize(config, res);
  }
};


////////////////////////////////////////////////////////////////////////////////
//
// Create IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleCreateIRodsObject : public Operation
{
public:
  HandleCreateIRodsObject() : Operation("icreate",
                                        "icreate <IRODS_PATH>: create a new PID for irods object\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() < 1 ||
       (config.args->getValue().size() - 1) % 2 != 0)
    {
      std::cerr << "at least three arguments required: "
                << "1. irods path,"
                << "2. key value,"
                << "3. value" << std::endl;
      return 8;
    }
    return 8;
  }
  
  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto args = config.args->getValue();
    auto itr = args.begin();
    std::string path(*itr);
    ++itr;
    auto res = client->create(path, listToPairs(itr, args.end()));
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Move IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleMoveIRodsObject : public Operation
{
public:
  HandleMoveIRodsObject(): Operation("imove",
                                       "imove <OLD_PATH> <NEW_PATH>: move PID for irods object\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 2)
    {
      std::cerr << "exactly two arguments (irods old path / new path) required for move operation" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto res = client->move(config.args->getValue()[0], config.args->getValue()[1]);
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Delete IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleDeleteIRodsObject : public Operation
{
public:
  HandleDeleteIRodsObject(): Operation("idelete",
                                       "idelete <PATH>: remove path for irods object\n") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 1)
    {
      std::cerr << "exactly one argument (irods path) required for delete operation" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto res = client->remove(config.args->getValue()[0]);
    return finalize(config, res);
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// Get IRods Object
//
////////////////////////////////////////////////////////////////////////////////
class HandleGetIRodsObject : public Operation
{
public:
  HandleGetIRodsObject(): Operation("iget",
                                    "iget <PATH>: get info for given irods path\n"
                                    "iget <PATH> <JSONPATH>: get key from irods meta dataobject") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 1 &&
       config.args->getValue().size() != 2)
    {
      std::cerr << "one or two arguments required: 1. irods path and 2. handle TYPE  (optional)" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto res = client->get(config.args->getValue()[0]);
    if(!res.success)
    {
      std::cerr << res << std::endl;
      return 8;
    }
    else if(config.args->getValue().size() == 2)
    {
      std::cout << surfsara::handle::extractValueByType(res.data, config.args->getValue()[1]) << std::endl;
    }
    else
    {
      return finalize(config, res);
    }
  }
};

class HandleSetIRodsMetaData : public Operation
{
public:
  HandleSetIRodsMetaData(): Operation("iset",
                                      "iset <PATH> (<TYPE> <VALUE>)+: set the key for meta data entry") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() < 3 ||
       (config.args->getValue().size() - 1) % 2 != 0)
    {
      std::cerr << "at least three arguments required: "
                << "1. irods path,"
                << "2. handle type,"
                << "3. value" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto args = config.args->getValue();
    auto itr = args.begin();
    std::string path(*itr);
    ++itr;
    auto res = client->set(path, listToPairs(itr, args.end()));
    return finalize(config, res);
  }
};

class HandleUnsetIRodsMetaData : public Operation
{
public:
  HandleUnsetIRodsMetaData(): Operation("iunset",
                                        "iunset <PATH> <JSONPATH>: removes key from meta data entry") {}
  virtual int parse(Config & config) override
  {
    if(config.args->getValue().size() != 2)
    {
      std::cerr << "two arguments required: 1. irods path, 2. handle type" << std::endl;
      return 8;
    }
    return 0;
  }

  virtual int exec(Config & config) override
  {
    auto client = config.makeIRodsHandleClient();
    auto res = client->unset(config.args->getValue()[0],
                             {config.args->getValue()[1]});
    return finalize(config, res);
  }
};

///////////////////////////////////////////////////////////////////////////////
int finalize(const Config & config, const surfsara::handle::Result & res)
{
  if(!res.success)
  {
    std::cerr << res << std::endl;
    return 8;
  }
  if(config.verbose->isSet())
  {
    std::cout << res << std::endl;
  }
  auto result = surfsara::ast::parseJson(res.curlResult.body);
  auto jsonString = surfsara::ast::formatJson(result, true);
  if(config.output->isSet())
  {
    std::ofstream ofs(config.output->getValue().c_str(), std::ofstream::out);
    ofs << jsonString << std::endl;
    ofs.close();
  }
  std::cout << jsonString << std::endl;
  return 0;
}

inline bool checkLookupParameters(const Config & config)
{
  bool ok = true;
  if(!config.lookup_url->isSet())
  {
    std::cerr << "required argument --lookup_url" << std::endl;
    ok = false;
  }
  if(!config.lookup_port->isSet())
  {
    std::cerr << "required argument --lookup_port" << std::endl;
    ok = false;
  }
  return ok;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, const char ** argv)
{
  surfsara::handle::Config cfg({
      std::make_shared<HandleCreate>(),
      std::make_shared<HandleGet>(),
      std::make_shared<HandleMove>(),
      std::make_shared<HandleResolve>(),
      std::make_shared<HandleLookup>(),
      std::make_shared<HandleDelete>(),
      std::make_shared<HandleDeletePid>(),
      std::make_shared<HandleCreateIRodsObject>(),
      std::make_shared<HandleMoveIRodsObject>(),
      std::make_shared<HandleDeleteIRodsObject>(),
      std::make_shared<HandleGetIRodsObject>(),
      std::make_shared<HandleSetIRodsMetaData>(),
      std::make_shared<HandleUnsetIRodsMetaData>()});
  
  auto op = cfg.parseArgs(argc, argv);
  if(!op)
  {
    if(cfg.help->isSet())
    {
      return 0;
    }
    else
    {
      return 8;
    }
  }
  auto ret = op->parse(cfg);
  if(ret != 0)
  {
    cfg.parser.printHelp(std::cout);
    return ret;
  }
  else
  {
    return op->exec(cfg);
  }
  return 0;
}


