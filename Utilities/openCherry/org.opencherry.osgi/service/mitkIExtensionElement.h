#ifndef MITKIEXTENSIONELEMENT_H_
#define MITKIEXTENSIONELEMENT_H_


#include "mitkDll.h"
#include "mitkMacros.h"

#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"

#include "../mitkIClassLoader.h"

#include <vector>
#include <string>

namespace mitk {

struct MITK_API IExtensionElement : public Poco::RefCountedObject
{
  
  MITK_SMART_OBJECT(IExtensionElement);
  
public:
  
  template<class C>
  C* CreateExecutableExtension(const std::string& base, const std::string& propertyName)
  {
    const std::string className(this->GetAttribute(propertyName));
    return m_ClassLoader->LoadClass<C>(m_Contributor, base, className);
  }
  
  virtual std::string GetAttribute(const std::string& name) const = 0;
  
  virtual void GetChildren(std::vector<IExtensionElement::Ptr>& children) = 0;
  virtual void GetChildren(std::string name, std::vector<IExtensionElement::Ptr>& children) = 0;
  
  virtual std::string GetValue() const = 0;
  
  virtual std::string GetName() const = 0;
  virtual IExtensionElement::Ptr GetParent() const = 0;
  
  virtual const std::string& GetContributor() const = 0;
  
  virtual ~IExtensionElement() {};
  
  
protected:
  IClassLoader* m_ClassLoader;
  std::string m_Contributor;
  
};

}  // namespace mitk


#endif /*MITKIEXTENSIONELEMENT_H_*/
