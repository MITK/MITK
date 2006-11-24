#include "QmitkFunctionalityFactory.h"

QmitkFunctionalityFactory& QmitkFunctionalityFactory::GetInstance() {
  static QmitkFunctionalityFactory* instance = new QmitkFunctionalityFactory();
  return *instance;  
}; 

