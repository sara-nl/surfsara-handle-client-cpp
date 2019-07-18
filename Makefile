INCLUDE = -ICatch2/single_include/ -ICliArgs/include -Iinclude -Ijson-parser-cpp/include/ -Iinclude
CXX = g++
CXXFLAGS = -std=c++11 -O2
CXXLIBS = -lcurl

all:  test_handle handle

-include handle.dep
-include test_handle.dep
-include test_util.dep
-include test_permssions.dep

handle: src/handle.cpp ${DEP}
	${CXX} ${CXXFLAGS} ${INCLUDE} src/handle.cpp ${CXXLIBS} -o handle
	${CXX} ${CXXFLAGS} ${INCLUDE} -MM -MT handle -MF handle.dep src/handle.cpp

test_handle: test_handle.o test_util.o test_permissions.o unit_test/test_main.cpp
	${CXX} ${CXXFLAGS} ${INCLUDE} test_handle.o test_util.o test_permissions.o unit_test/test_main.cpp ${CXXLIBS} -o test_handle

test_handle.o:
	${CXX} ${CXXFLAGS} ${INCLUDE} -c unit_test/test_handle.cpp -o test_handle.o
	${CXX} ${CXXFLAGS} ${INCLUDE} -MM -MT test_handle.o -MF test_handle.dep unit_test/test_handle.cpp


test_util.o:
	${CXX} ${CXXFLAGS} ${INCLUDE} -c unit_test/test_util.cpp -o test_util.o
	${CXX} ${CXXFLAGS} ${INCLUDE} -MM -MT test_util.o -MF test_util.dep unit_test/test_util.cpp

test_permissions.o:
	${CXX} ${CXXFLAGS} ${INCLUDE} -c unit_test/test_permissions.cpp -o test_permissions.o
	${CXX} ${CXXFLAGS} ${INCLUDE} -MM -MT test_permissions.o -MF test_permissions.dep unit_test/test_permissions.cpp


clean:
	rm -f test_util.o
	rm -f test_handle.o
	rm -f test_permissions.o
	rm -f test_handle
	rm -f test_util.dep
	rm -f test_handle.dep
	rm -f test_permissions.dep
	rm -f handle.dep
	rm -f handle

