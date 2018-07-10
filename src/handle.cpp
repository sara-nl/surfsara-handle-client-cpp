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

class HandleProgramArgs
{
public:
  HandleProgramArgs();
  inline int parse(int argc, const char ** argv);
  inline std::vector<surfsara::handle::Operation> createUpdateOperations();

  Cli::Parser parser;
  std::shared_ptr<Cli::PositionalValue<std::string>> operation;
  std::shared_ptr<Cli::PositionalValue<std::string>> url;
  std::shared_ptr<Cli::PositionalValue<std::string>> uuid;
  std::shared_ptr<Cli::PositionalMultipleValue<std::string>> json;
  std::shared_ptr<Cli::Flag> verbose;
  std::shared_ptr<Cli::Flag> help;
  std::shared_ptr<Cli::Value<long>> port;
};

inline HandleProgramArgs::HandleProgramArgs()
  : parser("CLI tool to perform PID operations")
{
  operation = parser.addPositionalValue<std::string>("OPERATION",
                                                     Cli::Doc("Operation: "
                                                              "create / update / get / delete"));
  url = parser.addPositionalValue<std::string>("URL",
                                               Cli::Doc("Url (url/prefix/suffix) "
                                                        "suffix not required for create operation"));
  
  uuid = parser.addPositionalValue<std::string>("UUID",
                                                Cli::Doc("uuid"));
  
  json = parser.addPositionalMultipleValue<std::string>("JSON", Cli::Doc("json data"));
  help = parser.addFlag('h', "help", Cli::Doc("show help"));
  verbose = parser.addFlag('v', "verbose", Cli::Doc("verbose output"));
  port = parser.addValue<long>('p', "port", Cli::Doc("port"));
}


inline std::vector<surfsara::handle::Operation> HandleProgramArgs::createUpdateOperations()
{
  std::vector<surfsara::handle::Operation> ret;
  for(auto keyValuePair : json->getValue())
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
  if(help->isSet())
  {
    parser.printHelp(std::cout);
    return 0;
  }
  if(!operation->isSet())
  {
    std::cerr << "operation is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  if(!url->isSet())
  {
    std::cerr << "url is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  if(!uuid->isSet())
  {
    std::cerr << "UUID is required" << std::endl;
    parser.printHelp(std::cerr);
    return 8;
  }
  
  if((operation->getValue() == "create") || (operation->getValue() == "update"))
  {
    if(json->getValue().size() != 1)
    {
      std::cerr << "exactly one JSON object is required" << std::endl;
      parser.printHelp(std::cerr);
      return 8;
    }
  }
  if(operation->getValue() == "delete")
  {
    if(json->getValue().size() > 0)
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
  std::string uuid = args.uuid->getValue() == "auto" ? std::string("") : args.uuid->getValue();
  HandleClient client(args.url->getValue(), uuid,
                      {
                        surfsara::curl::Verbose(args.verbose->isSet()),
                        surfsara::curl::Port(args.port->getValue()),
                        surfsara::curl::Header({
                            "Content-Type:application/json",
                            "Authorization: Handle clientCert=\"true\""
                              })
                      });

  if(args.operation->getValue() == "create")
  {
    Node res = client.create(args.createUpdateOperations());
    std::cout << surfsara::ast::formatJson(res, true) << std::endl;
  }
  else if(args.operation->getValue() == "update")
  {
    Node res = client.update(args.createUpdateOperations());
    std::cout << surfsara::ast::formatJson(res, true) << std::endl;
  }
  else if(args.operation->getValue() == "delete")
  {
    Node res = client.remove();
    std::cout << surfsara::ast::formatJson(res, true) << std::endl;
  }
  return 0;
}
