#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:
   
  DataStorageReference(DataStorage::Pointer dataStorage, DataTree::Pointer dataTree, bool isDefault = false);
  
  DataStorage::Pointer GetDataStorage() const;
  DataTree::Pointer GetDataTree() const;
  
  bool IsDefault() const;
  
  std::string GetLabel() const;
  void SetLabel(const std::string& label);
    
  bool operator==(const IDataStorageReference* o) const;
  
private:
  
  bool m_Default;
  std::string m_Label;
  DataStorage::Pointer m_DataStorage;
  DataTree::Pointer m_DataTree;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
