#include "QmitkViewPart.h"

#include <cherryPlatform.h>
#include <mitkIDataStorageService.h>

QmitkViewPart::QmitkViewPart()
 : m_HandlesMultipleDataStorages(false)
{
  
}
  
void QmitkViewPart::SetHandleMultipleDataStorages(bool multiple)
{
  m_HandlesMultipleDataStorages = multiple;
}

bool QmitkViewPart::HandlesMultipleDataStorages() const
{
  return m_HandlesMultipleDataStorages;
}
  
mitk::DataStorage::Pointer 
QmitkViewPart::GetDataStorage() const
{
  mitk::IDataStorageService::Pointer service = 
    cherry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
  
  if (service.IsNotNull())
  {
    if (m_HandlesMultipleDataStorages)
      return service->GetActiveDataStorage()->GetDataStorage();
    else
      return service->GetDefaultDataStorage()->GetDataStorage();
  }
  
  return 0;
}
