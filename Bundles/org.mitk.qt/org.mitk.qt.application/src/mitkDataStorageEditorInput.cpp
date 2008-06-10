#include "mitkDataStorageEditorInput.h"

#include <org.opencherry.osgi/cherryPlatform.h>
#include <org.mitk.core.services/src/mitkIDataStorageService.h>

bool mitkDataStorageEditorInput::Exists()
{
  return true;
}
  
std::string mitkDataStorageEditorInput::GetName()
{
  return "DataStorage Scene";
}
  
std::string mitkDataStorageEditorInput::GetToolTipText()
{
  return "";
}

mitk::IDataStorageReference::Pointer 
mitkDataStorageEditorInput::GetDataStorageReference()
{
  if (m_DataStorageRef.IsNull())
  {
    cherry::ServiceRegistry& serviceRegistry = cherry::Platform::GetServiceRegistry();
    mitk::IDataStorageService::Pointer dataService = serviceRegistry.GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
    if (dataService.IsNull()) return 0;
    m_DataStorageRef = dataService->CreateDataStorage(this->GetName());
  }
  
  return m_DataStorageRef;
}
