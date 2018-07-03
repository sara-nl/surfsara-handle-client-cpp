GCC= g++ -std=c++11
INC=  -I/usr/include/irods/ 
INC+= -I/app/irods-4.1.11/iRODS/server/re/include/
INC+= -I/app/irods-4.1.11/iRODS/lib/core/include/
INC+= -I/app/irods-4.1.11/iRODS/lib/api/include/
INC+= -I/app/irods-4.1.11/iRODS/server/core/include/
INC+= -I/app/irods-4.1.11/iRODS/server/icat/include/
INC+= -I/app/irods-4.1.11/iRODS/server/drivers/include/
INC+= -I/app/irods-4.1.11/iRODS/lib/api/include/

all: libmsi_wordcount.so  test_wordcount
.PONY: all

libmsi_wordcount.so: libmsi_wordcount.cpp libmsi_wordcount.h
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -o libmsi_wordcount.so libmsi_wordcount.cpp -Wno-deprecated

test_wordcount: test_wordcount.cpp libmsi_wordcount.h
	${GCC} -I../Catch2/single_include/ test_wordcount.cpp -o test_wordcount

install:
	cp libmsi_wordcount.so /var/lib/irods/plugins/microservices/

clean:
	rm -f test_wordcount
	rm -f libmsi_wordcount.so



