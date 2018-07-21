#pragma once
#include <memory>
#include <string>
#include <exception>
#include <fstream>
#include <streambuf>
#include <memory>
#include <cli.h>
#include <surfsara/json_parser.h>
#include <surfsara/json_format.h>
#include <surfsara/handle_client.h>
#include <surfsara/reverse_lookup_client.h>
#include <surfsara/irods_handle_client.h>

namespace surfsara
{
  namespace handle
  {
    class Config;

    class Operation
    {
    public:
      Operation(const std::string & _name,
                const std::string & _help) : name(_name), help(_help) {}
      virtual int parse(Config & args) = 0;
      virtual int exec(Config & args) = 0;
      
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

    class Config
    {
    public:
      Config(const std::vector<std::shared_ptr<Operation>> & op={});

      inline void parseJson(const std::string & filename, bool verbose=false);
      inline std::shared_ptr<Operation> parseArgs(int argc, const char ** argv);
      
      inline std::shared_ptr<HandleClient> makeHandleClient() const;
      inline std::shared_ptr<ReverseLookupClient> makeReverseLookupClient() const;
      inline std::shared_ptr<IRodsHandleClient> makeIRodsHandleClient() const;

      // only available in CLI tools
      std::shared_ptr<Cli::PositionalValue<std::string>>         operation;
      std::shared_ptr<Cli::PositionalValue<std::string>>         handle;
      std::shared_ptr<Cli::PositionalMultipleValue<std::string>> args;
      std::shared_ptr<Cli::Flag>                                 help;
      std::shared_ptr<Cli::Value<std::string>>                   output;
      std::shared_ptr<Cli::Value<std::string>>                   configfile;

      // verbose
      std::shared_ptr<Cli::Flag>               verbose;

      // handle
      std::shared_ptr<Cli::Value<std::string>> handle_url;
      std::shared_ptr<Cli::Value<long>>        handle_port;
      std::shared_ptr<Cli::Value<std::string>> handle_key;
      std::shared_ptr<Cli::Value<std::string>> handle_cert;
      std::shared_ptr<Cli::Value<std::string>> handle_caCert;
      std::shared_ptr<Cli::Flag>               handle_insecure;
      std::shared_ptr<Cli::Flag>               handle_passphrase;
      std::shared_ptr<Cli::Value<std::string>> handle_prefix;

      // lookup
      std::shared_ptr<Cli::Value<std::string>> lookup_url;
      std::shared_ptr<Cli::Value<long>>        lookup_port;
      std::shared_ptr<Cli::Value<std::string>> lookup_user;
      std::shared_ptr<Cli::Value<std::string>> lookup_password;
      std::shared_ptr<Cli::Flag>               lookup_insecure;
      std::shared_ptr<Cli::Value<long>>        lookup_limit;
      std::shared_ptr<Cli::Value<long>>        lookup_page;

      // irods arguments
      std::shared_ptr<Cli::Value<std::string>> irods_server;
      std::shared_ptr<Cli::Value<long>>        irods_port;
      std::shared_ptr<Cli::Value<std::string>> irods_url_prefix;
      std::shared_ptr<Cli::Value<std::string>> irods_webdav_prefix;
      std::shared_ptr<Cli::Value<long>>        irods_webdav_port;

      Cli::Parser parser;
    private:
      std::vector<std::shared_ptr<Operation>> operations;
      template<typename T>
      inline void addOperation();
      inline std::string getOperationsString() const;
      inline std::string getOperationsHelp() const;

      inline void setArgument(std::shared_ptr<Cli::Argument> arg,
                              const surfsara::ast::Node & node);
    };
  }
}

namespace surfsara
{
  namespace handle
  {
    inline Config::Config(const std::vector<std::shared_ptr<Operation>> & op)
      : operations(op),
        parser("CLI tool to perform PID operations")
    {
      operation = parser.addPositionalValue<std::string>("OPERATION", Cli::Doc(getOperationsString() + "\n" + getOperationsHelp()));
      args                = parser.addPositionalMultipleValue<std::string>("ARGS", Cli::Doc("operation specific arguments"));
      help                = parser.addFlag('h', "help", Cli::Doc("show help"));
      output              = parser.addValue<std::string>('o', "output", Cli::Doc("Write resulting JSON document to file"));
      configfile          = parser.addValue<std::string>('c', "config", Cli::Doc("Read configuration from file"));

      verbose             = parser.addFlag("verbose", Cli::Doc("verbose outout"));
      // handle server options
      handle_url          = parser.addValue<std::string>("handle_url", Cli::Doc("Url to handle server "));
      handle_port         = parser.addValue<long>('p', "handle_port", Cli::Doc("port"));
      handle_key          = parser.addValue<std::string>("handle_key", Cli::Doc("key file (PEM)"));
      handle_cert         = parser.addValue<std::string>("handle_cert", Cli::Doc("certificate file (PEM)"));
      handle_caCert       = parser.addValue<std::string>("handle_cacert", Cli::Doc("CA certificate to verify peer against"));
      handle_passphrase   = parser.addFlag("handle_passphrase", Cli::Doc("key file requires passphrase, ask for it"));
      handle_insecure     = parser.addFlag("handle_insecure", Cli::Doc("Allow insecure server connections when using SSL"));
      handle_prefix       = parser.addValue<std::string>("handle_prefix", Cli::Doc("Prefix"));

      // reverse lookup arguments
      lookup_url          = parser.addValue<std::string>("lookup_url", Cli::Doc("Url to reverse lookup server "));
      lookup_port         = parser.addValue<long>("lookup_port", Cli::Doc("Port for reverse lookup server"));
      lookup_user         = parser.addValue<std::string>("lookup_user", Cli::Doc("User for reverse lookup server "));
      lookup_password     = parser.addValue<std::string>("lookup_password", Cli::Doc("Password for reverse lookup server "));
      lookup_insecure     = parser.addFlag("lookup_insecure", Cli::Doc("Allow insecure server connections with reverse lookup server when using SSL"));
      lookup_limit        = parser.addValue<long>("lookup_limit", Cli::Doc("Pagination Limit"));
      lookup_page         = parser.addValue<long>("lookup_page", Cli::Doc("Pagination Page"));

      // irods setting
      irods_server        = parser.addValue<std::string>("irods_server", Cli::Doc("FQDN or IP of the ICat server"));
      irods_port          = parser.addValue<long>("irods_port", Cli::Doc("Port of the ICat server, default 1247"));
      irods_url_prefix    = parser.addValue<std::string>("irods_url_prefix", Cli::Doc("Prefix for the irods server, default: irods://{irods_server}"));
      irods_webdav_prefix = parser.addValue<std::string>("irods_webdav_prefix", Cli::Doc("Prefix for the webdav server (Optional)"));
      irods_webdav_port   = parser.addValue<long>("irods_webdav_port", Cli::Doc("Webdav server port, default: 80"));
    }

    inline void Config::parseJson(const std::string & filename, bool _verbose)
    {
      using Node = surfsara::ast::Node;
      using Object = surfsara::ast::Object;
      using String = surfsara::ast::String;
      if(verbose)
      {
        std::cout << "read config from file " << filename << std::endl;
      }
      std::ifstream ist(filename.c_str());
      if(!ist.good())
      {
        throw std::runtime_error(std::string("failed to read config file:") + filename);
      }
      std::string str((std::istreambuf_iterator<char>(ist)),
                      std::istreambuf_iterator<char>());
      auto node = surfsara::ast::parseJson(str);
      if(node.isA<surfsara::ast::Object>())
      {
        for(const char * group : {"handle", "lookup", "irods"})
        {
          if(node.as<Object>().has(group))
          {
            auto subnode = node.as<Object>()[group];
            if(subnode.isA<Object>())
            {
              subnode.as<Object>().forEach([this, group](const String & key, const Node & node){
                  setArgument(parser.getArgument(std::string(group) + std::string("_") + key), node);
              });
            }
          }
        }
      }
      else
      {
        throw std::logic_error(std::string("invalid json object: exepcted object, given ") + node.typeName());
      }
      if(verbose)
      {
        verbose->setValue(true);
      }
    }

    inline std::shared_ptr<Operation> Config::parseArgs(int argc, const char ** argv)
    {
      std::shared_ptr<Operation> selectedOp;
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
        parseJson(configfile->getValue());
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


    inline std::shared_ptr<HandleClient> Config::makeHandleClient() const
    {
      std::string passphrase;
      return std::make_shared<HandleClient>(handle_url->getValue(),
                                            std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                              surfsara::curl::Verbose(verbose->isSet()),
                                              surfsara::curl::Port(handle_port->getValue()),
                                              surfsara::curl::SslPem(handle_cert->getValue(),
                                                                     handle_key->getValue(),
                                                                     handle_insecure->isSet(),
                                                                     passphrase,
                                                                     handle_caCert->getValue())},
                                            verbose->isSet());
    }

    inline std::shared_ptr<ReverseLookupClient> Config::makeReverseLookupClient() const
    {
      return std::make_shared<ReverseLookupClient>(lookup_url->getValue(),
                                                   handle_prefix->getValue(),
                                                   std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                                     surfsara::curl::Verbose(verbose->isSet()),
                                                     surfsara::curl::Port(lookup_port->getValue()),
                                                       surfsara::curl::HttpAuth(lookup_user->getValue(),
                                                                                lookup_password->getValue(),
                                                                                lookup_insecure->isSet())},
                                                   (lookup_limit->isSet() ? lookup_limit->getValue() : 100),
                                                   (lookup_page->isSet() ? lookup_page->getValue() : 0),
                                                                     verbose->isSet());
    }

    inline std::shared_ptr<IRodsHandleClient> Config::makeIRodsHandleClient() const
    {
      return std::make_shared<IRodsHandleClient>(makeHandleClient(),
                                                 makeReverseLookupClient(),
                                                 IRodsConfig(
                                                             irods_url_prefix->getValue(),
                                                             irods_server->getValue(),
                                                             handle_prefix->getValue(),
                                                             irods_port->getValue(),
                                                             irods_webdav_prefix->getValue(),
                                                             irods_webdav_port->getValue()));
    }

    template<typename T>
    inline void Config::addOperation()
    {
      operations.push_back(std::make_shared<T>());
    }

    inline std::string Config::getOperationsString() const
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

    inline std::string Config::getOperationsHelp() const
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

    inline void Config::setArgument(std::shared_ptr<Cli::Argument> arg,
                                    const surfsara::ast::Node & node)
    {
      using Boolean = surfsara::ast::Boolean;
      using Null = surfsara::ast::Null;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      if(arg && !arg->isSet() && !node.isA<Null>())
      {
        if(std::dynamic_pointer_cast<Cli::Flag>(arg))
        {
          if(node.isA<Boolean>())
          {
            std::dynamic_pointer_cast<Cli::Flag>(arg)->setValue(node.as<Boolean>());
          }
          else
          {
            throw std::logic_error("execpected boolean, given " +
                                   node.typeName() + ": " +
                                   surfsara::ast::formatJson(node));
          }
        }
        else if(std::dynamic_pointer_cast<Cli::Value<std::string>>(arg))
        {
          if(node.isA<String>())
          {
            std::dynamic_pointer_cast<Cli::Value<std::string>>(arg)->setValue(node.as<String>());
          }
          else
          {
            throw std::logic_error("execpected String, given " +
                                   node.typeName() + ": " +
                                   surfsara::ast::formatJson(node));
          }
        }
        else if(std::dynamic_pointer_cast<Cli::Value<long>>(arg))
        {
          if(node.isA<Integer>())
          {
            std::dynamic_pointer_cast<Cli::Value<long>>(arg)->setValue(node.as<Integer>());
          }
          else
          {
            throw std::logic_error("execpected Integer, given " +
                                   node.typeName() + ": " +
                                   surfsara::ast::formatJson(node));
          }
        }
      }
    }

  } // handle
} // surfsara
