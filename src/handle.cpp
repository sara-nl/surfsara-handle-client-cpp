#include <surfsara/curl.h>
#include <iostream>
#include <cli.h>

using namespace surfsara::curl;

//////////////////////////////////
// adding handle
//
// ./handle put prefix/suffix/ '[{ index: 1, type: type, data: {"abc": 1}}]'
//
//////////////////////////////////
// get handle
//
// ./handle get url/prefix/suffix                    # all
// ./handle get url/prefix/suffix/{INDEX}            # specific index
// ./handle get url/prefix/suffix/{INDEX}/type       # type for specific index
// ./handle get url/prefix/suffix/{INDEX}/data       # data object for specific index
// ./handle get url/prefix/suffix/{INDEX}/data/field # field of data object
// ./handle get url/prefix/suffix/{INDEX}/data/pa/th # field of suboject
//
//////////////////////////////////
// update
//
// all indices
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


//////////////////////////////////
// delete
//
// complete handle
// ./handle delete url/prefix/suffix/
//
// one index 
// ./handle delete url/prefix/suffix/{INDEX}
//
// one field index 
// ./handle delete url/prefix/suffix/{INDEX}/data/field

int main(int argc, const char ** argv)
{
  Cli::Parser parser("CLI tool to perform PID operations");
  std::string operation;
  std::string url;
  std::string data;
  bool verbose = false;
  parser.add(Cli::Value<std::string>::make(operation, Cli::Doc("Operation: put/get/delete")));
  parser.add(Cli::Value<std::string>::make(url, Cli::Doc("Url")));
  parser.add(Cli::Value<std::string>::make(data, Cli::Doc("json data")));
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
  Curl curl{
        Url("http://127.0.0.1:5000/21.T12995/test"),
        Data("{\"values\": [ { \"dfdf\": 1 } ]}"),
        Verbose(verbose),
        Header({"Content-Type:application/json",
              "Authorization: Handle clientCert=\"true\""})};
  auto res = curl.request();
  std::cout << res.body << std::endl;
  return 0;
}
