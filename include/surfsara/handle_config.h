#pragma once
#include <memory>
#include <string>
#include <exception>
#include <fstream>
#include <streambuf>
#include <cli.h>
#include <surfsara/json_parser.h>
#include <surfsara/json_format.h>

namespace surfsara
{
  namespace handle
  {
    class Config
    {
    public:
      inline void registerArguments(Cli::Parser & parser);
      inline void parseJson(const std::string & filename, bool verbose=false);

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

    private:
      inline void setArgument(std::shared_ptr<Cli::Argument> arg,
                              const surfsara::ast::Node & node);
    };
  }
}

namespace surfsara
{
  namespace handle
  {
    inline void Config::registerArguments(Cli::Parser & parser)
    {
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

    inline void Config::parseJson(const std::string & filename, bool verbose)
    {
      using Node = surfsara::ast::Node;
      using Object = surfsara::ast::Object;
      using String = surfsara::ast::String;
      Cli::Parser parser;
      registerArguments(parser);
      if(verbose)
      {
        std::cout << "read config from file " << filename << std::endl;
      }
      std::ifstream ist(filename.c_str());
      std::string str((std::istreambuf_iterator<char>(ist)),
                      std::istreambuf_iterator<char>());
      auto node = surfsara::ast::parseJson(str);
      if(node.isA<surfsara::ast::Object>())
      {
        for(const char * group : {"handle", "lookup", "irods2"})
        {
          if(node.as<Object>().has(group))
          {
            auto subnode = node.as<Object>()[group];
            if(subnode.isA<Object>())
            {
              subnode.as<Object>().forEach([this, group, &parser](const String & key, const Node & node){
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
  }
}
