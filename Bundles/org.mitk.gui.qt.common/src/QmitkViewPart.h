#ifndef QMITKVIEWPART_H_
#define QMITKVIEWPART_H_

#include <cherryQtViewPart.h>

#include <mitkDataStorage.h>

class QmitkViewPart : cherry::QtViewPart
{
  
public:
  cherryClassMacro(QmitkViewPart)
  
  QmitkViewPart();
  
protected:
  
  void SetHandleMultipleDataStorages(bool multiple);
  bool HandlesMultipleDataStorages() const;
  
  mitk::DataStorage::Pointer GetDataStorage() const;
  
private:
  
  bool m_HandlesMultipleDataStorages;
};

#endif /*QMITKVIEWPART_H_*/
