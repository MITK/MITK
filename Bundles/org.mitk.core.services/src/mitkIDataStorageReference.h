#ifndef MITKIDATASTORAGEREFERENCE_H_
#define MITKIDATASTORAGEREFERENCE_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "mitkCorePluginDll.h"

#include <mitkDataStorage.h>

namespace mitk
{

struct MITK_CORE_PLUGIN IDataStorageReference : public cherry::Object
{
  cherryClassMacro(IDataStorageReference);
 
  
  virtual DataStorage::Pointer GetDataStorage() = 0;
  virtual DataTree::Pointer GetDataTree() = 0;
  
  virtual std::string GetLabel() = 0;
  virtual void SetLabel(const std::string& label) = 0;
  
  virtual bool operator==(const IDataStorageReference* o) = 0;
};

}

#endif /*MITKIDATASTORAGEREFERENCE_H_*/
