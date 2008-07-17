#ifndef MITKIDATASTORAGEREFERENCE_H_
#define MITKIDATASTORAGEREFERENCE_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "mitkCoreServicesPluginDll.h"

#include <mitkDataStorage.h>

namespace mitk
{

struct MITK_CORE_SERVICES_PLUGIN IDataStorageReference : public cherry::Object
{
  cherryClassMacro(IDataStorageReference);
 
  
  virtual DataStorage::Pointer GetDataStorage() const = 0;
  virtual DataTree::Pointer GetDataTree() const = 0;
  
  virtual bool IsDefault() const = 0;
  
  virtual std::string GetLabel() const = 0;
  virtual void SetLabel(const std::string& label) = 0;
  
  virtual bool operator==(const IDataStorageReference* o) const = 0;
};

}

#endif /*MITKIDATASTORAGEREFERENCE_H_*/
