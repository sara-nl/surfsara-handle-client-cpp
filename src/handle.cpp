#include <surfsara/curl.h>
#include <surfsara/handle_client.h>
#include <surfsara/reverse_lookup_client.h>
#include <surfsara/handle_validation.h>
#include <surfsara/json_format.h>
#include <surfsara/json_parser.h>
#include <surfsara/ast.h>
#include <surfsara/util.h>
#include <iostream>
#include <cli.h>

using HandleClient = surfsara::handle::HandleClient;
using ValidationError = surfsara::handle::ValidationError;
using Node = surfsara::ast::Node;

class HandleProgramArgs;

class HandleOperation
{
public:
  HandleOperation(const std::string & _name,
                  const std::string & _help) : name(_name), help(_help) {}
  virtual ~HandleOperation() {}

  bool checkLookupParameters(HandleProgramArgs & args);
  HandleClient createHandleClient(HandleProgramArgs & args);
  int finalize(const HandleProgramArgs & args, const surfsara::handle::Result & res);
  virtual int parse(HandleProgramArgs & args) = 0;
  virtual int exec(HandleProgramArgs & args) = 0;

  inline const std::string& getName() const
  {
    return name;
  }

  inline const std::string& getHelp() const
  {
    return help;
  }

private:
  std::string name;
  std::string help;
};

//////////////////////////////////////////////////////////////////////////////////////////

class HandleCreate : public HandleOperation
{
public:
  HandleCreate() : HandleOperation("create",
                                   "create <PREFIX> <PATH>: create a new PID for irods object\n"
                                   "create --json <PREFIX> <JSON>: create a new PID from json\n") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleGet : public HandleOperation
{
public:
  HandleGet(): HandleOperation("get",
                               "get <HANDLE>: get PID\n") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleUpdate : public HandleOperation
{
public:
  HandleUpdate(): HandleOperation("update",
                                  "update <HANDLE> <PATH>: update PID for irods object\n"
                                  "update --json <HANDLE> <JSON>: update PID (only inidices that appear in the JSON doc)\n"
                                  "update --json --replace <HANDLE> <JSON>: replace PID\n") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleResolve : public HandleOperation
{
public:
  HandleResolve():  HandleOperation("resolve",""){}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleLookup : public HandleOperation
{
public:
  HandleLookup(): HandleOperation("lookup",
                                  "lookup <PREFIX> <KEY>=<VALUE> <KEY>=<VALUE> ...: find a handle by reverse lookup\n") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleDelete : public HandleOperation
{
public:
  HandleDelete(): HandleOperation("delete",
                                  "delete <HANDLE> <KEY> <KEY>: delete a key from PID\n") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};

class HandleDeletePid : public HandleOperation
{
public:
  HandleDeletePid(): HandleOperation("delete_pid",
                                     "delete_pid <HANDLE>: delete full PID") {}
  virtual int parse(HandleProgramArgs & args) override;
  virtual int exec(HandleProgramArgs & args) override;
};


class HandleProgramArgs
{
public:
  HandleProgramArgs();
  template<typename T>
  inline void addOperation();
  std::string getOperationsString() const;
  std::string getOperationsHelp() const;
  inline std::shared_ptr<HandleOperation> parse(int argc, const char ** argv);

  Cli::Parser parser;
  std::shared_ptr<Cli::PositionalValue<std::string>> operation;
  std::shared_ptr<Cli::PositionalValue<std::string>> handle;
  std::shared_ptr<Cli::PositionalMultipleValue<std::string>> args;
  std::shared_ptr<Cli::Flag>               verbose;
  std::shared_ptr<Cli::Flag>               json;
  std::shared_ptr<Cli::Flag>               help;
  std::shared_ptr<Cli::Value<std::string>> url;
  std::shared_ptr<Cli::Value<long>>        port;
  std::shared_ptr<Cli::Value<std::string>> key;
  std::shared_ptr<Cli::Value<std::string>> cert;
  std::shared_ptr<Cli::Value<std::string>> caCert;
  std::shared_ptr<Cli::Flag>               insecure;
  std::shared_ptr<Cli::Flag>               passphrase;

  std::shared_ptr<Cli::Value<std::string>> lookup_url;
  std::shared_ptr<Cli::Value<long>>        lookup_port;
  std::shared_ptr<Cli::Value<std::string>> lookup_user;
  std::shared_ptr<Cli::Value<std::string>> lookup_password;
  std::shared_ptr<Cli::Flag>               lookup_insecure;
  std::shared_ptr<Cli::Value<long>>        lookup_limit;
  std::shared_ptr<Cli::Value<long>>        lookup_page;

private:
  std::vector<std::shared_ptr<HandleOperation>> operations;
};

inline HandleProgramArgs::HandleProgramArgs() : parser("CLI tool to perform PID operations")
{
  addOperation<HandleCreate>();
  addOperation<HandleGet>();
  addOperation<HandleUpdate>();
  addOperation<HandleResolve>();
  addOperation<HandleLookup>();
  addOperation<HandleDelete>();
  addOperation<HandleDeletePid>();
  operation = parser.addPositionalValue<std::string>("OPERATION",
                                                     Cli::Doc(getOperationsString() + "\n" + getOperationsHelp()));
  handle = parser.addPositionalValue<std::string>("HANDLE",
                                                  Cli::Doc("prefix/suffix or only prefix for create operation"));
  
  args             = parser.addPositionalMultipleValue<std::string>("VALUES", Cli::Doc("arguments or json string"));
  help             = parser.addFlag('h', "help", Cli::Doc("show help"));
  verbose          = parser.addFlag('v', "verbose", Cli::Doc("verbose output"));
  json             = parser.addFlag('j', "json", Cli::Doc("accepts json instead of high level operations."));
  url              = parser.addValue<std::string>("url", Cli::Doc("Url to handle server "));
  port             = parser.addValue<long>('p', "port", Cli::Doc("port"));
  key              = parser.addValue<std::string>("key", Cli::Doc("key file (PEM)"));
  cert             = parser.addValue<std::string>("cert", Cli::Doc("certificate file (PEM)"));
  caCert           = parser.addValue<std::string>("cacert", Cli::Doc("CA certificate to verify peer against"));
  passphrase       = parser.addFlag("passphrase", Cli::Doc("key file requires passphrase, ask for it"));
  insecure         = parser.addFlag("insecure", Cli::Doc("Allow insecure server connections when using SSL"));

  lookup_url       = parser.addValue<std::string>("lookup_url", Cli::Doc("Url to reverse lookup server "));
  lookup_port      = parser.addValue<long>("lookup_port", Cli::Doc("Port for reverse lookup server"));
  lookup_user      = parser.addValue<std::string>("lookup_user", Cli::Doc("User for reverse lookup server "));
  lookup_password  = parser.addValue<std::string>("lookup_password", Cli::Doc("Password for reverse lookup server "));
  lookup_insecure  = parser.addFlag("lookup_insecure", Cli::Doc("Allow insecure server connections with reverse lookup server when using SSL"));
  lookup_limit     = parser.addValue<long>("lookup_limit", Cli::Doc("Pagination Limit"));
  lookup_page     = parser.addValue<long>("lookup_limit", Cli::Doc("Pagination Page"));
}
    
template<typename T>
inline void HandleProgramArgs::addOperation()
{
  operations.push_back(std::make_shared<T>());
}

std::string HandleProgramArgs::getOperationsString() const
{
  std::string ret;
  for(auto op : operations)
  {
    if(!ret.empty())
    {
      ret += "|";
    }
    ret += op->getName();
  }
  return ret;
}

std::string HandleProgramArgs::getOperationsHelp() const
{
  std::string ret;
  for(auto op : operations)
  {
    if(!ret.empty())
    {
      ret += "\n";
    }
    ret += op->getHelp();
  }
  return ret;
}

inline std::shared_ptr<HandleOperation> HandleProgramArgs::parse(int argc, const char ** argv)
{
  std::shared_ptr<HandleOperation> selectedOp;
  std::vector<std::string> err;
  if(!parser.parse(argc, argv, err))
  {
    for(auto line : err)
    {
      std::cerr << line << std::endl;
    }
    parser.printHelp(std::cerr);
    return selectedOp;
  }
  if(help->isSet())
  {
    parser.printHelp(std::cout);
    return selectedOp;
  }
  if(!operation->isSet())
  {
    std::cerr << "operation is required" << std::endl;
    parser.printHelp(std::cerr);
    return selectedOp;
  }
  for(auto op : operations)
  {
    if(op->getName() == operation->getValue())
    {
      selectedOp = op;
      break;
    }
  }
  if(selectedOp)
  {
    //return selectedOp->parse(*this);
    return selectedOp;
  }
  else
  {
    std::cerr << "invalid operation " << operation->getValue() << std::endl;
    parser.printHelp(std::cerr);
    return selectedOp;
  }
}

///////////////////////////////////////////////////////////////////////////////
bool HandleOperation::checkLookupParameters(HandleProgramArgs & args)
{
  bool ok = true;
  std::vector<std::string> req{"lookup_url", "lookup_port", "lookup_user", "lookup_password"};
  for(auto p: req)
  {
    if(!args.parser.isSet(p))
    {
      std::cerr << "required argument --" << p << std::endl;
      ok = false;
    }
  }
  return ok;
}

HandleClient HandleOperation::createHandleClient(HandleProgramArgs & args)
{
  std::string passphrase;
  if(args.passphrase->isSet())
  {
    passphrase = surfsara::util::readPassord();
  }
  return HandleClient(args.url->getValue(),
                      args.handle->getValue(),
                      std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                        surfsara::curl::Verbose(args.verbose->isSet()),
                        surfsara::curl::Port(args.port->getValue()),
                          surfsara::curl::SslPem(args.cert->getValue(),
                                                 args.key->getValue(),
                                                 args.insecure->isSet(),
                                                 passphrase,
                                                 args.caCert->getValue())},
                      args.verbose->isSet());
}

int HandleOperation::finalize(const HandleProgramArgs & args,
                              const surfsara::handle::Result & res)
{
  if(!res.success)
  {
    std::cerr << res << std::endl;
    return 8;
  }
  if(args.verbose->isSet())
  {
    std::cout << res << std::endl;
  }
  std::cout << surfsara::ast::formatJson(surfsara::ast::parseJson(res.curlResult.body), true) << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int HandleCreate::parse(HandleProgramArgs & args)
{
  if(args.args->getValue().size() != 1)
  {
    std::cerr << "exactly on argument expected for operation create" << std::endl;
    return 8;
  }

  return 0;
}

int HandleCreate::exec(HandleProgramArgs & args)
{
  HandleClient client(createHandleClient(args));
  surfsara::handle::Result res;
  if(args.json->isSet())
  {
    Node node(surfsara::ast::parseJson(args.args->getValue().front()));
    res = client.create(node);
  }
  else
  {
  }
  return finalize(args, res);
}

////////////////////////////////////////////////////////////////////////////////
int HandleGet::parse(HandleProgramArgs & args)
{
  if(args.args->getValue().size() > 0)
  {
    std::cerr << "no arguments allowed for get operation" << std::endl;
    return 8;
  }
  return 0;
}
  
int HandleGet::exec(HandleProgramArgs & args)
{
  HandleClient client(createHandleClient(args));
  surfsara::handle::Result res;
  res = client.get();
  return finalize(args, res);
}

////////////////////////////////////////////////////////////////////////////////  
int HandleUpdate::parse(HandleProgramArgs & args)
{
  if(args.args->getValue().size() != 1)
  {
    std::cerr << "exactly on argument expected for operation create" << std::endl;
    return 8;
  }
  return 0;
}

int HandleUpdate::exec(HandleProgramArgs & args)
{
  HandleClient client(createHandleClient(args));
  surfsara::handle::Result res;
  if(args.json->isSet())
  {
    Node node(surfsara::ast::parseJson(args.args->getValue().front()));
    res = client.update(node);
  }
  else
  {
  }
  return finalize(args, res);
}

////////////////////////////////////////////////////////////////////////////////
int HandleResolve::parse(HandleProgramArgs & args)
{
  return 0;
}

int HandleResolve::exec(HandleProgramArgs & args)
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Lookup
//
////////////////////////////////////////////////////////////////////////////////
int HandleLookup::parse(HandleProgramArgs & args)
{
  int ret = 0;
  if(!args.handle->isSet())
  {
    std::cerr << "missing HANDLE (prefix)" << std::endl;
    ret = 8;
  }
  if(!checkLookupParameters(args))
  {
    ret = 8;
  }
  return ret;
}

int HandleLookup::exec(HandleProgramArgs & args)
{
  surfsara::handle::ReverseLookupClient client(args.lookup_url->getValue(),
                                               args.handle->getValue(),
                                               std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                                 surfsara::curl::Verbose(args.verbose->isSet()),
                                                 surfsara::curl::Port(args.lookup_port->getValue()),
                                                 surfsara::curl::HttpAuth(args.lookup_user->getValue(),
                                                                          args.lookup_password->getValue(),
                                                                          args.lookup_insecure->isSet())},
                                               (args.lookup_limit->isSet() ? args.lookup_limit->getValue() : 100),
                                               (args.lookup_page->isSet() ? args.lookup_page->getValue() : 0),
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

////////////////////////////////////////////////////////////////////////////////
int HandleDelete::parse(HandleProgramArgs & args)
{
  if(args.args->getValue().size() == 0)
  {
    std::cerr << "required at least index to be removed" << std::endl;
    return 8;
  }
  return 0;
}

int HandleDelete::exec(HandleProgramArgs & args)
{
  HandleClient client(createHandleClient(args));
  std::vector<int> indices;
  for(auto arg : args.args->getValue())
  {
    indices.push_back(surfsara::util::fromString<int>(arg));
  }
  auto res = client.removeIndices(indices);
  return finalize(args, res);
}

////////////////////////////////////////////////////////////////////////////////
int HandleDeletePid::parse(HandleProgramArgs & args)
{
  if(args.args->getValue().size() > 0)
  {
    std::cerr << "no arguments allowed for get operation" << std::endl;
    return 8;
  }
}

int HandleDeletePid::exec(HandleProgramArgs & args)
{
  HandleClient client(createHandleClient(args));
  auto res = client.remove();
  return finalize(args, res);
}

int main(int argc, const char ** argv)
{
  HandleProgramArgs args;
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
