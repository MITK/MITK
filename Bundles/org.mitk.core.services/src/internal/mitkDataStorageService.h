#ifndef MITKDATASTORAGESERVICE_H_
#define MITKDATASTORAGESERVICE_H_

#include "../mitkIDataStorageService.h"

namespace mitk {

class DataStorageService : public IDataStorageService
{
public:
  
  bool IsA(const std::type_info& type);

  const std::type_info& GetType() const;

  IDataStorageReference::Pointer CreateDataStorage(const std::string& label);
    
  std::vector<IDataStorageReference::Pointer> GetDataStorageReferences();
  
private:
  
  std::vector<IDataStorageReference::Pointer> m_DataStorageReferences;
};

}

#endif /*MITKDATASTORAGESERVICE_H_*/
