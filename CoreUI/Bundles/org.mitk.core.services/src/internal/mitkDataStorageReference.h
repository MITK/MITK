#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:
   
  DataStorageReference(DataStorage::Pointer dataStorage, bool isDefault = false);
  
  DataStorage::Pointer GetDataStorage() const;
  
  bool IsDefault() const;
  
  std::string GetLabel() const;
  void SetLabel(const std::string& label);
    
  bool operator==(const IDataStorageReference* o) const;
  
private:
  
  bool m_Default;
  std::string m_Label;
  DataStorage::Pointer m_DataStorage;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
