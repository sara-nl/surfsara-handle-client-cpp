#include <string>
#include <iostream>
#include <fstream>
#include <apiHeaderAll.h>
#include <msParam.h>
#include <irods_ms_plugin.hpp>
#include <dataObjOpen.h>
#include <dataObjRead.h>
#include <dataObjClose.h>
#include "libmsi_wordcount.h"

#include "reGlobalsExtern.hpp"
#include "rsGlobalExtern.hpp"
#include "rcGlobalExtern.h"



//#include <reGlobalsExtern.hpp>
extern "C" {
  double get_plugin_interface_version() {
    return 1.0;
  }

  int msi_wordcount(msParam_t* _inFileName, msParam_t* _outWordCount, ruleExecInfo_t* rei) {
    if (rei == NULL || rei->rsComm == NULL) {
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    if (_inFileName == NULL || _inFileName->type == NULL ||
        strcmp(_inFileName->type, STR_MS_T) != 0) {
      return (USER_PARAM_TYPE_ERR);
    }
    char * path = (char*)(_inFileName->inOutStruct);
    WordCounter wc;
    rsComm_t *rsComm = rei->rsComm;
    dataObjInp_t file;
    openedDataObjInp_t openedFile;
    bytesBuf_t bytesBuf;
    char buffer[10];
    int bytesRead;
    memset(&file, 0, sizeof (dataObjInp_t));
    memset(&openedFile, 0, sizeof (openedDataObjInp_t));
    memset(&bytesBuf, 0, sizeof (bytesBuf_t));
    bytesBuf.len = sizeof(buffer);
    bytesBuf.buf = buffer;
    strncpy(file.objPath, path, MAX_NAME_LEN);
    openedFile.l1descInx = rsDataObjOpen(rsComm, &file);
    openedFile.len = bytesBuf.len;
    if (openedFile.l1descInx < 0) {
      rodsLog(LOG_ERROR, "Cannot read iRODS object %s Status =  %d", path, openedFile.l1descInx);
      return OBJ_PATH_DOES_NOT_EXIST;
    }
    bytesRead = rsDataObjRead(rsComm, &openedFile, &bytesBuf);
    if (bytesRead < 0) {
      rodsLog(LOG_ERROR, "Problem reading iRODS object. Status =  %d", bytesRead);
      return FILE_READ_ERR;
    }
    while(bytesRead > 0) {
      wc.read((char*)bytesBuf.buf, bytesBuf.len);
      bytesRead = rsDataObjRead(rsComm, &openedFile, &bytesBuf);
    }
    int status = rsDataObjClose(rsComm, &openedFile);
    if (status < 0) {
      rodsLog(LOG_ERROR, "rsDataObjClose failed for %s, status = %d", path, status);
      return FILE_READ_ERR;
    }
    fillIntInMsParam(_outWordCount, wc.getNumWords());
    return 0;
  }


  extern irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(2);
    //msvc->add_operation("msi_wordcount", std::function<int(msParam_t*,
    //msParam_t*,
    //ruleExecInfo_t*)>(msi_wordcount_impl));
    msvc->add_operation("msi_wordcount", "msi_wordcount");

    return msvc;
  }
}
