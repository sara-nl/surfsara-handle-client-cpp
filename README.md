# Handle Client
A c++ header only library and an a CLI application.

## Compiling the CLI tool
1. Install required packages:
  * boost libraries
  * libcurl
2. pull in required submodules
```
git submodule update --recursive --init
```
3. compile 
```
make
```
This will produce 2 executable programs:
1. handle: The CLI tool to create / test handles
2. test_handle: The unit tests

## Using the CLI tool

### Preparation
__requirements:__ 
1. the certificate file and the private key PEM files for SSL authentification to the handle service.
2. the credentials for the reverse lookup server

Copy the configuration template set the passwords and location of PEM files:
```
cp config.json.template config.json
vi config.json
```

Note that all configuration parameters can be overwritten with command line arguments. Type 
```
./hanlde --help 
```
for more details.
