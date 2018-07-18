#pragma once
#include <string>
#include <surfsara/handle_client.h>
#include <surfsara/handle_config.h>
#include <cli.h>
#include <surfsara/util.h>

class HandleProgramArgs;

class HandleOperation
{
public:
  HandleOperation(const std::string & _name,
                  const std::string & _help) : name(_name), help(_help) {}

  bool checkLookupParameters(HandleProgramArgs & args);
  std::shared_ptr<surfsara::handle::HandleClient> createHandleClient(HandleProgramArgs & args);
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


class HandleProgramArgs
{
public:
  HandleProgramArgs();
  inline void registerArguments();
  
  template<typename T>
  inline void addOperation();
  
  std::string getOperationsString() const;
  std::string getOperationsHelp() const;
  inline std::shared_ptr<HandleOperation> parse(int argc, const char ** argv);

  std::shared_ptr<Cli::PositionalValue<std::string>> operation;
  std::shared_ptr<Cli::PositionalValue<std::string>> handle;
  std::shared_ptr<Cli::PositionalMultipleValue<std::string>> args;
  std::shared_ptr<Cli::Flag>               verbose;
  std::shared_ptr<Cli::Flag>               help;
  std::shared_ptr<Cli::Value<std::string>> output;
  std::shared_ptr<Cli::Value<std::string>> configfile;

  Cli::Parser parser;
  surfsara::handle::Config config;
private:
  std::vector<std::shared_ptr<HandleOperation>> operations;
};

////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////
bool HandleOperation::checkLookupParameters(HandleProgramArgs & args)
{
  bool ok = true;
  if(!args.config.lookup_url->isSet())
  {
    std::cerr << "required argument --lookup_url" << std::endl;
    ok = false;
  }
  if(!args.config.lookup_port->isSet())
  {
    std::cerr << "required argument --lookup_port" << std::endl;
    ok = false;
  }
  return ok;
}

std::shared_ptr<surfsara::handle::HandleClient> HandleOperation::createHandleClient(HandleProgramArgs & args)
{
  using HandleClient = surfsara::handle::HandleClient;
  std::string passphrase;
  if(args.config.handle_passphrase->isSet())
  {
    passphrase = surfsara::util::readPassord();
  }
  return std::make_shared<HandleClient>(args.config.handle_url->getValue(),
                                        std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                          surfsara::curl::Verbose(args.verbose->isSet()),
                                          surfsara::curl::Port(args.config.handle_port->getValue()),
                                          surfsara::curl::SslPem(args.config.handle_cert->getValue(),
                                          args.config.handle_key->getValue(),
                                                                 args.config.handle_insecure->isSet(),
                                                                 passphrase,
                                                                 args.config.handle_caCert->getValue())},
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
  auto result = surfsara::ast::parseJson(res.curlResult.body);
  auto jsonString = surfsara::ast::formatJson(result, true);
  if(args.output->isSet())
  {
    std::ofstream ofs(args.output->getValue().c_str(), std::ofstream::out);
    ofs << jsonString << std::endl;
    ofs.close();
  }
  std::cout << jsonString << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////
inline HandleProgramArgs::HandleProgramArgs() : parser("CLI tool to perform PID operations")
{
}

inline void HandleProgramArgs::registerArguments()
{
  operation = parser.addPositionalValue<std::string>("OPERATION",
                                                     Cli::Doc(getOperationsString() + "\n" + getOperationsHelp()));
  args                = parser.addPositionalMultipleValue<std::string>("ARGS", Cli::Doc("operation specific arguments"));
  help                = parser.addFlag('h', "help", Cli::Doc("show help"));
  verbose             = parser.addFlag('v', "verbose", Cli::Doc("verbose output"));
  output              = parser.addValue<std::string>('o', "output", Cli::Doc("Write resulting JSON document to file"));
  configfile          = parser.addValue<std::string>('c', "config", Cli::Doc("Read configuration from file"));
  config.registerArguments(parser);
}

template<typename T>
inline void HandleProgramArgs::addOperation()
{
  operations.push_back(std::make_shared<T>());
}

inline std::string HandleProgramArgs::getOperationsString() const
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

inline std::string HandleProgramArgs::getOperationsHelp() const
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
  if(configfile->isSet())
  {
    config.parseJson(configfile->getValue(), verbose->isSet());
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


