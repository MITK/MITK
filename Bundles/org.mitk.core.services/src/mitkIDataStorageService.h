#ifndef MITKIDATASTORAGESERVICE_H_
#define MITKIDATASTORAGESERVICE_H_

#include <service/cherryService.h>

#include "mitkCoreServicesPluginDll.h"
#include "mitkIDataStorageReference.h"

#include <string>

namespace mitk
{

struct MITK_CORE_SERVICES_PLUGIN IDataStorageService : public cherry::Service
{
  cherryClassMacro(IDataStorageService);
  
  static const std::string ID;
  
  virtual IDataStorageReference::Pointer CreateDataStorage(const std::string& label = "") = 0;
  virtual std::vector<IDataStorageReference::Pointer> GetDataStorageReferences() = 0;
  
};

}

#endif /*MITKIDATASTORAGESERVICE_H_*/
