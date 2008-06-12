#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:
   
  DataStorageReference(DataStorage::Pointer dataStorage, DataTree::Pointer dataTree);
  
  DataStorage::Pointer GetDataStorage();
  DataTree::Pointer GetDataTree();
  
  std::string GetLabel();
  void SetLabel(const std::string& label);
    
  bool operator==(const IDataStorageReference* o);
  
private:
  
  std::string m_Label;
  DataStorage::Pointer m_DataStorage;
  DataTree::Pointer m_DataTree;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
