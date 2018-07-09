#include <surfsara/curl.h>
#include <surfsara/handle_client.h>
#include <surfsara/handle_validation.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>
#include <surfsara/ast.h>
#include <iostream>
#include <cli.h>

using HandleClient = surfsara::handle::HandleClient;
using ValidationError = surfsara::handle::ValidationError;
using Node = surfsara::ast::Node;

struct HandleProgramArgs
{
  std::string operation;
  std::string url;
  std::string uuid;
  std::vector<std::string> json;
  bool verbose;
  inline HandleProgramArgs();
  inline int parse(int argc, const char ** argv);
  inline std::vector<surfsara::handle::Operation> createUpdateOperations();
};

inline HandleProgramArgs::HandleProgramArgs()
  : verbose(false)
{
}

inline std::vector<surfsara::handle::Operation> HandleProgramArgs::createUpdateOperations()
{
  std::vector<surfsara::handle::Operation> ret;
  for(auto keyValuePair : json)
  {
    std::size_t p = keyValuePair.find('=');
    if(p == std::string::npos)
    {
      throw std::runtime_error("invalid format, expected: {PATH}={JSON}");
    }
    else
    {
      std::string json(keyValuePair.begin() + p + 1,
                       keyValuePair.end());
      std::string path(keyValuePair.begin(), keyValuePair.begin() + p);
      
      if(json == "delete")
      {
        ret.push_back(surfsara::handle::Delete(path));
      }
      else
      {
        ret.push_back(surfsara::handle::Update(path,
                                               surfsara::ast::parseJson(json)));
      }
    }
  }
  return ret;
}

inline int HandleProgramArgs::parse(int argc, const char ** argv)
{
  Cli::Parser parser("CLI tool to perform PID operations");
  auto argOp = parser.add(Cli::Value<std::string>::make(operation,
                                                        Cli::Doc("Operation: "
                                                                 "create / update / get / delate")));
  auto argUrl = parser.add(Cli::Value<std::string>::make(url,
                                                         Cli::Doc("Url (url/prefix/suffix) "
                                                                  "suffix not required for create operation")));
  auto argUuid = parser.add(Cli::Value<std::string>::make(uuid,
                                                          Cli::Doc("uuid")));


  auto argJson = parser.add(Cli::MultipleValue<std::string>::make(json,
                                                                  Cli::Doc("json data")));
  parser.add(Cli::Flag::make('h', "help", Cli::Doc("show help")));
  parser.add(Cli::Flag::make('v', "verbose", Cli::Doc("verbose output")));
  verbose = parser.isSet("verbose");
  std::vector<std::string> err;
  if(!parser.parse(argc, argv, err))
  {
    for(auto line : err)
    {
      std::cerr << line << std::endl;
    }
    parser.printHelp(std::cerr);
    return 8;
  }
  if(parser.isSet("help"))
  {
    parser.printHelp(std::cout);
    return 0;
  }
  if(!argOp->isSet())
  {
    std::cerr << "operation is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  if(!argUrl->isSet())
  {
    std::cerr << "url is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  if(!argUuid->isSet())
  {
    std::cerr << "UUID is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  
  if(operation == "create")
  {
    if(uuid == "auto")
    {
      uuid = "";
    }
    if(json.size() != 1)
    {
      std::cerr << "exactly on JSON object is required" << std::endl;
      parser.printHelp(std::cerr);
      return 8;
    }
  }
  else if(operation == "update")
  {
    if(json.size() < 1)
    {
      std::cerr << "at least one object required" << std::endl;
      parser.printHelp(std::cerr);
      return 8;
    }
  }
  if(operation == "delete")
  {
    if(json.size() > 0)
    {
      std::cerr << "no JSON argument allowed for delete operation" << std::endl;
      parser.printHelp(std::cerr);
      return 8;
    }
  }
  return 0;
}

int main(int argc, const char ** argv)
{
  HandleProgramArgs args;
  int ret = args.parse(argc, argv);
  if(ret != 0)
  {
    return ret;
  }
  HandleClient client(args.url,
                      {
                        surfsara::curl::Verbose(args.verbose),
                        surfsara::curl::Header({
                            "Content-Type:application/json",
                            "Authorization: Handle clientCert=\"true\""
                              })
                      });

  if(args.operation == "create")
  {
    Node node = surfsara::ast::parseJson(args.json[0]);
    Node res = client.create(node, args.uuid);

  }
  else if(args.operation == "update")
  {
    Node res = client.update(args.uuid, args.createUpdateOperations());
    std::cout << surfsara::ast::formatJson(res, true) << std::endl;
  }
  else if(args.operation == "delete")
  {
    Node res = client.remove(args.uuid);
    std::cout << surfsara::ast::formatJson(res, true) << std::endl;
  }
  return 0;
}
