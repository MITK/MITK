#include "mitkDataStorageEditorInput.h"

#include <cherryPlatform.h>
#include <mitkIDataStorageService.h>

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
