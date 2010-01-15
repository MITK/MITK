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

#ifndef CHERRYTYPEEXTENSIONMANAGER_H_
#define CHERRYTYPEEXTENSIONMANAGER_H_

#include "../cherryIPropertyTester.h"

#include "cherryTypeExtension.h"
#include "cherryPropertyCache.h"

#include "cherryProperty.h"

#include <cherryIConfigurationElement.h>

#include <string>
#include <typeinfo>
#include <map>

namespace cherry {

class TypeExtensionManager { // implements IRegistryChangeListener {

private:
  std::string fExtensionPoint;

  static const std::string TYPE;

  class NULL_PROPERTY_TESTER_ : public IPropertyTester
  {
  public:
    bool Handles(const std::string&  /*namespaze*/, const std::string&  /*property*/)
    {
      return false;
    }
    bool IsInstantiated()
    {
      return true;
    }
    bool IsDeclaringPluginActive()
    {
      return true;
    }
    IPropertyTester* Instantiate()
    {
      return this;
    }
    bool Test(Object::Pointer, const std::string& /*property*/,
        std::vector<Object::Pointer>&  /*args*/, Object::Pointer  /*expectedValue*/)
    {
      return false;
    }
  };

  static const NULL_PROPERTY_TESTER_ NULL_PROPERTY_TESTER;

  /*
   * Map containing all already created type extension object.
   */
  std::map<std::string, TypeExtension::Pointer> fTypeExtensionMap;

  /*
   * Table containing mapping of class name to configuration element
   */
  std::map<std::string, std::vector<IConfigurationElement::Pointer> >* fConfigurationElementMap;

  /*
   * A cache to give fast access to the last 1000 method invocations.
   */
  PropertyCache* fPropertyCache;

public:

  TypeExtensionManager(const std::string& extensionPoint);
  ~TypeExtensionManager();

  Property::Pointer GetProperty(Object::Pointer receiver,
      const std::string& namespaze, const std::string& method);

  /*synchronized*/Property::Pointer GetProperty(Object::Pointer receiver,
      const std::string& namespaze, const std::string& method, bool forcePluginActivation);

protected:

  friend class TypeExtension;

  /*
   * This method doesn't need to be synchronized since it is called
   * from withing the getProperty method which is synchronized
   */
  /* package */TypeExtension::Pointer Get(const std::string& type);

  /*
   * This method doesn't need to be synchronized since it is called
   * from withing the getProperty method which is synchronized
   */
  /* package */void LoadTesters(std::vector<IPropertyTester::Pointer>& result, const std::string& typeName);

  //  public void registryChanged(IRegistryChangeEvent event) {
  //    IExtensionDelta[] deltas= event.getExtensionDeltas(ExpressionPlugin.getPluginId(), fExtensionPoint);
  //    if (deltas.length > 0) {
  //      initializeCaches();
  //    }
  //  }

private:

  /*synchronized*/void InitializeCaches();
};

}

#endif /*CHERRYTYPEEXTENSIONMANAGER_H_*/
