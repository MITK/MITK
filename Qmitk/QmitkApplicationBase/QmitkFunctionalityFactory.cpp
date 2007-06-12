#include "QmitkFunctionalityFactory.h"

QmitkFunctionalityFactory& QmitkFunctionalityFactory::GetInstance() {
  static QmitkFunctionalityFactory* instance = new QmitkFunctionalityFactory();
  return *instance;  
}; 
QmitkFunctionalityFactory::CreateFunctionalityPtr QmitkFunctionalityFactory::GetCreateFunctionalityPtrByName(const std::string name) const {
  CreateFunctionalityPtrMap::const_iterator it = m_RegisteredFunctionalities.find(name);
  if ( it != m_RegisteredFunctionalities.end()) {
    return it->second;
  }
  else
  {
    return NULL;
  }
}    

