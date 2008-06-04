/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef CHERRYIEXTENSIONELEMENT_H_
#define CHERRYIEXTENSIONELEMENT_H_


#include "../cherryOSGiDll.h"

#include "../cherryBundleLoader.h"
#include "../cherryPlatformException.h"

#include <vector>
#include <string>

namespace cherry {

struct IExtension;

struct CHERRY_OSGI IConfigurationElement : public Object
{
  
  cherryClassMacro(IConfigurationElement);
  
public:
  
  typedef std::vector<IConfigurationElement::Pointer> vector;
  
  template<class C>
  C* CreateExecutableExtension(const std::string& base, const std::string& propertyName)
  {
    std::string className;
    if (this->GetAttribute(propertyName, className))
    {
      try
      {
        C* cl = m_ClassLoader->LoadClass<C>(m_Contributor, base, className);
        return cl;
      }
      catch (Poco::Exception* e)
      {
        std::cout << "Error loading class: " << e->displayText() << std::endl;
        exit(1);
      }
    }
    
    throw CoreException("Missing attribute", propertyName);
  }
  
  virtual bool GetAttribute(const std::string& name, std::string& value) const = 0;
  
  virtual const std::vector<IConfigurationElement::Pointer> GetChildren() const = 0;
  virtual const std::vector<IConfigurationElement::Pointer> GetChildren(const std::string& name) const = 0;
  
  virtual std::string GetValue() const = 0;
  
  virtual std::string GetName() const = 0;
  virtual const IConfigurationElement* GetParent() const = 0;
  
  virtual const std::string& GetContributor() const = 0;
  virtual const IExtension* GetDeclaringExtension() const = 0;
  
  virtual ~IConfigurationElement() {};
  
  
protected:
  BundleLoader* m_ClassLoader;
  std::string m_Contributor;
  
};

}  // namespace cherry


#endif /*CHERRYIEXTENSIONELEMENT_H_*/
