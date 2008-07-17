#ifndef MITKDATASTORAGESERVICE_H_
#define MITKDATASTORAGESERVICE_H_

#include "../mitkIDataStorageService.h"

namespace mitk {

class DataStorageService : public IDataStorageService
{
public:
  
  DataStorageService();
  
  bool IsA(const std::type_info& type);

  const std::type_info& GetType() const;

  IDataStorageReference::Pointer CreateDataStorage(const std::string& label);
    
  std::vector<IDataStorageReference::Pointer> GetDataStorageReferences() const;
  
  IDataStorageReference::Pointer GetDefaultDataStorage() const;
    
  IDataStorageReference::Pointer GetActiveDataStorage() const;
  void SetActiveDataStorage(IDataStorageReference::Pointer dataStorageRef);
  
private:
  
  IDataStorageReference::Pointer CreateDefaultDataStorage();
  
  IDataStorageReference::Pointer m_ActiveDataStorageRef;
  IDataStorageReference::Pointer m_DefaultDataStorageRef;
  std::vector<IDataStorageReference::Pointer> m_DataStorageReferences;
};

}

#endif /*MITKDATASTORAGESERVICE_H_*/
