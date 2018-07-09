INCLUDE = -ICatch2/single_include/ -ICliArgs/include -Iinclude -Ijson-parser-cpp/include/ -Iinclude
CXX = g++
CXXFLAGS = -std=c++11
CXXLIBS = -lcurl

all: handle test_handle


handle: src/handle.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} src/handle.cpp ${CXXLIBS} -o handle

test_handle: src/test_handle.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} src/test_handle.cpp ${CXXLIBS} -o test_handle
