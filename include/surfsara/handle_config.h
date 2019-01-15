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
#pragma once
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
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
#include <surfsara/handle_profile.h>


namespace surfsara
{
#if defined(__clang__)
  namespace ast
  {
    std::istream & operator>>(std::istream & ist, Node & n);
    std::ostream & operator<<(std::ostream & ost, Node & n);
  }
#endif

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

      std::vector<std::pair<std::string, std::string>>
      listToPairs(std::vector<std::string>::const_iterator itr,
                  std::vector<std::string>::const_iterator end)
      {
        if((end - itr) % 2)
        {
          throw std::logic_error("cannot construct list of pairs from uneven list");
        }
        std::vector<std::pair<std::string, std::string>> kvpairs;
        while(itr != end)
        {
          std::pair<std::string, std::string> kvpair;
          kvpair.first = *itr;
          ++itr;
          kvpair.second = *itr;
          ++itr;
          kvpairs.push_back(kvpair);
        }
        return kvpairs;
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
      std::shared_ptr<Cli::Flag>               curl_verbose;

      // handle
      std::shared_ptr<Cli::Value<std::string>> handle_url;
      std::shared_ptr<Cli::Value<long>>        handle_port;
      std::shared_ptr<Cli::Value<std::string>> handle_key;
      std::shared_ptr<Cli::Value<std::string>> handle_cert;
      std::shared_ptr<Cli::Value<std::string>> handle_caCert;
      std::shared_ptr<Cli::Flag>               handle_insecure;
      std::shared_ptr<Cli::Flag>               handle_passphrase;
      std::shared_ptr<Cli::Value<std::string>> handle_prefix;
      std::shared_ptr<Cli::Value<surfsara::ast::Node>> handle_profile;
      std::shared_ptr<Cli::Value<long>>                handle_index_from;
      std::shared_ptr<Cli::Value<long>>                handle_index_to;

      // lookup
      std::shared_ptr<Cli::Value<std::string>> lookup_url;
      std::shared_ptr<Cli::Value<long>>        lookup_port;
      std::shared_ptr<Cli::Value<std::string>> lookup_prefix;
      std::shared_ptr<Cli::Value<std::string>> lookup_password;
      std::shared_ptr<Cli::Flag>               lookup_insecure;
      std::shared_ptr<Cli::Value<long>>        lookup_limit;
      std::shared_ptr<Cli::Value<long>>        lookup_page;
      std::shared_ptr<Cli::Flag>               lookup_before_create;
      std::shared_ptr<Cli::Value<std::string>> lookup_key;
      std::shared_ptr<Cli::Value<std::string>> lookup_value;


      // irods arguments
      std::shared_ptr<Cli::Value<std::string>> irods_server;
      std::shared_ptr<Cli::Value<long>>        irods_port;
      std::shared_ptr<Cli::Value<std::string>> irods_url_prefix;
      std::shared_ptr<Cli::Value<std::string>> irods_webdav_prefix;
      std::shared_ptr<Cli::Value<long>>        irods_webdav_port;

      Cli::Parser parser;

      std::map<std::string, std::string> parameters;
    private:
      std::vector<std::shared_ptr<Operation>> operations;
      long index_from;
      long index_to;

      template<typename T>
      inline void addOperation();
      inline std::string getOperationsString() const;
      inline std::string getOperationsHelp() const;

      inline void setArgument(const std::string & group,
                              const std::string & key,
                              const surfsara::ast::Node & node);
      inline void updateParameters();
    };
  }
}

inline std::istream & operator>>(std::istream & ist, surfsara::ast::Node & n);
inline std::ostream & operator<<(std::ostream & ost, const surfsara::ast::Node & n);

namespace surfsara
{
  namespace handle
  {
    inline Config::Config(const std::vector<std::shared_ptr<Operation>> & op)
      : operations(op),
        parser("CLI tool to perform PID operations")
    {
      index_from = 2;
      index_to = 100;

      operation = parser.addPositionalValue<std::string>("OPERATION", Cli::Doc(getOperationsString() + "\n" + getOperationsHelp()));
      args                = parser.addPositionalMultipleValue<std::string>("ARGS", Cli::Doc("operation specific arguments"));
      help                = parser.addFlag('h', "help", Cli::Doc("show help"));
      output              = parser.addValue<std::string>('o', "output", Cli::Doc("Write resulting JSON document to file"));
      configfile          = parser.addValue<std::string>('c', "config", Cli::Doc("Read configuration from file"));

      verbose             = parser.addFlag("verbose", Cli::Doc("verbose outout"));
      curl_verbose        = parser.addFlag("curl_verbose", Cli::Doc("verbose libcurl output"));

      // handle server options
      handle_url          = parser.addValue<std::string>("handle_url", Cli::Doc("Url to handle server "));
      handle_port         = parser.addValue<long>('p', "handle_port", Cli::Doc("port"));
      handle_key          = parser.addValue<std::string>("handle_key", Cli::Doc("key file (PEM)"));
      handle_cert         = parser.addValue<std::string>("handle_cert", Cli::Doc("certificate file (PEM)"));
      handle_caCert       = parser.addValue<std::string>("handle_cacert", Cli::Doc("CA certificate to verify peer against"));
      handle_passphrase   = parser.addFlag("handle_passphrase", Cli::Doc("key file requires passphrase, ask for it"));
      handle_insecure     = parser.addFlag("handle_insecure", Cli::Doc("Allow insecure server connections when using SSL"));
      handle_prefix       = parser.addValue<std::string>("handle_prefix", Cli::Doc("Prefix"));
      handle_profile      = parser.addValue<surfsara::ast::Node>("handle_profile", Cli::Doc("Handle profile"));
      /* @todo better solution for default value */
      handle_index_from   = parser.addValue<long>(index_from, "handle_index_from", Cli::Doc("Begin of free index range"));
      handle_index_to     = parser.addValue<long>(index_to, "handle_index_to", Cli::Doc("End of free index range index in range (index_from, index_to]"));
      
      // reverse lookup arguments
      lookup_url          = parser.addValue<std::string>("lookup_url", Cli::Doc("Url to reverse lookup server "));
      lookup_port         = parser.addValue<long>("lookup_port", Cli::Doc("Port for reverse lookup server"));
      lookup_prefix       = parser.addValue<std::string>("lookup_prefix", Cli::Doc("User for reverse lookup server "));
      lookup_password     = parser.addValue<std::string>("lookup_password", Cli::Doc("Password for reverse lookup server "));
      lookup_insecure     = parser.addFlag("lookup_insecure", Cli::Doc("Allow insecure server connections with reverse lookup server when using SSL"));
      lookup_limit        = parser.addValue<long>("lookup_limit", Cli::Doc("Pagination Limit"));
      lookup_page         = parser.addValue<long>("lookup_page", Cli::Doc("Pagination Page"));
      lookup_key          = parser.addValue<std::string>("lookup_key", Cli::Doc("The key that identifies the object in reverse lookup"));
      lookup_value         = parser.addValue<std::string>("lookup_value", Cli::Doc("The template of the value that identifies the object in reverse lookup"));
      lookup_before_create = parser.addFlag("lookup_before_create", Cli::Doc("Perform lookup query before creating a new handle"));

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
      if(_verbose)
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
        node.as<Object>().forEach([this](const String & group, const Node & subnode) {
            if(group == "handle" || group == "lookup" || group == "irods")
            {
              if(subnode.isA<Object>())
              {
                subnode.as<Object>().forEach([this, group](const String & key, const Node & n){
                    setArgument(group, key, n);
                  });
              }
            }
            else
            {
              setArgument("", group, subnode);
            }
          });
      }
      else
      {
        throw std::logic_error(std::string("invalid json object: exepcted object, given ") + node.typeName());
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
      updateParameters();
    }


    inline std::shared_ptr<HandleClient> Config::makeHandleClient() const
    {
      std::string passphrase;
      return std::make_shared<HandleClient>(handle_url->getValue(),
                                            std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                              surfsara::curl::Verbose(curl_verbose->isSet()),
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
                                                   lookup_prefix->getValue(),
                                                   std::vector<std::shared_ptr<surfsara::curl::BasicCurlOpt>>{
                                                     surfsara::curl::Verbose(curl_verbose->isSet()),
                                                     surfsara::curl::Port(lookup_port->getValue()),
                                                       surfsara::curl::HttpAuth(lookup_prefix->getValue(),
                                                                                lookup_password->getValue(),
                                                                                lookup_insecure->isSet())},
                                                   (lookup_limit->isSet() ? lookup_limit->getValue() : 100),
                                                   (lookup_page->isSet() ? lookup_page->getValue() : 0),
                                                   verbose->isSet());
    }

    inline std::shared_ptr<IRodsHandleClient> Config::makeIRodsHandleClient() const
    {
      using Null = surfsara::ast::Null;
      std::shared_ptr<HandleProfile> profile;
      auto node = handle_profile->getValue();
      if(node.isA<Null>())
      {
        profile = std::make_shared<HandleProfile>(parameters);
      }
      else
      {
        profile = std::make_shared<HandleProfile>(handle_profile->getValue(),
                                                  parameters,
                                                  index_from,
                                                  index_to);
      }
      return std::make_shared<IRodsHandleClient>(makeHandleClient(),
                                                 handle_prefix->getValue(),
                                                 makeReverseLookupClient(),
                                                 profile,
                                                 lookup_before_create->isSet(),
                                                 lookup_key->getValue(),
                                                 lookup_value->getValue());
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

    inline void Config::updateParameters()
    {
      for(auto arg : parser.getArguments())
      {
        if(!arg->isA<surfsara::ast::Node>() && arg->isSet())
        {
          std::string name = arg->getName();
          if(!name.empty() && name != "handle_profile")
          {
            std::transform(name.begin(), name.end(),
                           name.begin(), ::toupper);
            std::stringstream ost;
            arg->streamOut(ost);
            parameters[name] = ost.str();
          }
        }
      }
    }

    inline void Config::setArgument(const std::string & group,
                                    const std::string & key,
                                    const surfsara::ast::Node & node)
    {
      std::string name;
      if(group.empty())
      {
        name = key;
      }
      else
      {
        name = std::string(group) + std::string("_") + key;
      }
      const std::shared_ptr<Cli::Argument> arg = parser.getArgument(name);
      using Boolean = surfsara::ast::Boolean;
      using Null = surfsara::ast::Null;
      using String = surfsara::ast::String;
      using Integer = surfsara::ast::Integer;
      using Float = surfsara::ast::Float;
      if(!arg || !arg->isA<surfsara::ast::Node>())
      {
        std::transform(name.begin(), name.end(),
                       name.begin(), ::toupper);
        if(node.isA<String>())
        {
          parameters[name] = node.as<String>();          
        }
        else if(node.isA<Boolean>())
        {
          parameters[name] = std::to_string(node.as<Boolean>());
        }
        else if(node.isA<Integer>())
        {
          parameters[name] = std::to_string(node.as<Integer>());
        }
        else if(node.isA<Float>())
        {
          parameters[name] = std::to_string(node.as<Float>());
        }
      }
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
        else if(arg->isA<std::string>())
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
        else if(arg->isA<long>())
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
        else if(arg->isA<surfsara::ast::Node>())
        {
          std::dynamic_pointer_cast<Cli::Value<surfsara::ast::Node>>(arg)->setValue(node);
        }
      }
    }
  } // handle
} // surfsara


std::istream & operator>>(std::istream & ist, surfsara::ast::Node & n)
{
  std::string buff((std::istreambuf_iterator<char>(ist)),
                   std::istreambuf_iterator<char>());
  n = surfsara::ast::parseJson(buff);
  return ist;
}

std::ostream & operator<<(std::ostream & ost, const surfsara::ast::Node & n)
{
  surfsara::ast::formatJson(ost, n, true);
  return ost;
}
