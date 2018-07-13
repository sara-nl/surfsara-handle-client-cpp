INCLUDE = -ICatch2/single_include/ -ICliArgs/include -Iinclude -Ijson-parser-cpp/include/ -Iinclude
CXX = g++
CXXFLAGS = -std=c++11
CXXLIBS = -lcurl

all:  test_handle handle

DEP=	include/surfsara/curl.h \
	include/surfsara/curl_opt.h include/surfsara/handle_client.h \
	json-parser-cpp/include/surfsara/ast.h \
	json-parser-cpp/include/surfsara/impl/object.hpp \
	json-parser-cpp/include/surfsara/impl/array.hpp \
	json-parser-cpp/include/surfsara/impl/node.hpp \
	json-parser-cpp/include/surfsara/json_format.h \
	json-parser-cpp/include/surfsara/impl/json_format.hpp \
	json-parser-cpp/include/surfsara/json_parser.h \
	json-parser-cpp/include/surfsara/impl/json_parser.hpp \
	include/surfsara/handle_validation.h CliArgs/include/cli.h \
	CliArgs/include/cli_parser.h CliArgs/include/cli_flag.h \
	CliArgs/include/cli_argument.h CliArgs/include/cli_doc.h \
	CliArgs/include/cli_multiple_flag.h CliArgs/include/cli_value.h \
	CliArgs/include/cli_converter.h CliArgs/include/cli_positional_value.h \
	CliArgs/include/cli_multiple_value.h \
	CliArgs/include/cli_positional_multiple_value.h

handle: src/handle.cpp ${DEP}
	${CXX} ${CXXFLAGS} ${INCLUDE} src/handle.cpp ${CXXLIBS} -o handle

test_handle: src/test_handle.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} src/test_handle.cpp ${CXXLIBS} -o test_handle
