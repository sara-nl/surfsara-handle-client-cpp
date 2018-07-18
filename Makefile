INCLUDE = -ICatch2/single_include/ -ICliArgs/include -Iinclude -Ijson-parser-cpp/include/ -Iinclude
CXX = g++
CXXFLAGS = -std=c++11
CXXLIBS = -lcurl

all:  test_handle handle

-include handle.dep
-include test_handle.dep
-include test_util.dep

handle: src/handle.cpp src/handle_operation.h ${DEP}
	${CXX} ${CXXFLAGS} ${INCLUDE} src/handle.cpp ${CXXLIBS} -o handle
	${CXX} ${INCLUDE} -MM -MT handle -MF handle.dep src/handle.cpp

test_handle: test_handle.o test_util.o src/test_main.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} src/test_handle.cpp src/test_util.cpp src/test_main.cpp ${CXXLIBS} -o test_handle

test_handle.o:
	${CXX} ${CXXFLAGS} ${INCLUDE} -c src/test_handle.cpp -o test_handle.o
	${CXX} ${INCLUDE} -MM -MT test_handle.o -MF test_handle.dep src/test_handle.cpp

test_util.o:
	${CXX} ${CXXFLAGS} ${INCLUDE} -c src/test_util.cpp -o test_util.o
	${CXX} ${INCLUDE} -MM -MT test_util.o -MF test_util.dep src/test_util.cpp


clean:
	rm -f test_util.o
	rm -f test_handle.o
	rm -f test_handle
	rm -f test_util.dep
	rm -f test_handle.dep
	rm -f handle.dep
	rm -f handle

