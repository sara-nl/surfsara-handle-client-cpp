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
./handle --help 
```
for more details.

## Running tests
The unit tests can be exectued with the binary

```
./test_handle
```

To run the functional test with the integrated handle mockup:
1. Open a separete shell and and prepare the python virtual env.

```
cd handle-mockup
pipenv install -r requirements.txt
```

2. Execute the mockup server

```
pipenv run ./handle_mock.py
```

3. Go back to original shell and prepare the handle client configuration

```
cp config.json.mock config.json
```

4. Execute the shell script examples in the root directory

```
./create.sh
./get.sh
./delete.sh
./icreate.sh mykey myvalue
./iget.sh
./iset.sh mykey2 myvalue2
./iunset.sh mykey
./iget.sh
./idelete.sh
```

