/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYTYPEEXTENSIONMANAGER_H_
#define BERRYTYPEEXTENSIONMANAGER_H_

#include "berryIPropertyTester.h"

#include "berryTypeExtension.h"
#include "berryPropertyCache.h"

#include "berryProperty.h"

#include <berryIConfigurationElement.h>
#include <berryIRegistryEventListener.h>

#include <QHash>
#include <typeinfo>

namespace berry {

class TypeExtensionManager : private IRegistryEventListener
{

private:
  QString fExtensionPoint;

  static const QString TYPE;

  class NULL_PROPERTY_TESTER_ : public IPropertyTester
  {
  public:
    bool Handles(const QString&  /*namespaze*/, const QString&  /*property*/)
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
    bool Test(Object::ConstPointer, const QString& /*property*/,
              const QList<Object::Pointer>&  /*args*/, Object::Pointer  /*expectedValue*/)
    {
      return false;
    }
  };

  static const NULL_PROPERTY_TESTER_ NULL_PROPERTY_TESTER;

  /*
   * Map containing all already created type extension object.
   */
  QHash<QString, TypeExtension::Pointer> fTypeExtensionMap;

  /*
   * Table containing mapping of class name to configuration element
   */
  QHash<QString, QList<IConfigurationElement::Pointer> > fConfigurationElementMap;

  /*
   * A cache to give fast access to the last 1000 method invocations.
   */
  PropertyCache* fPropertyCache;

public:

  static bool DEBUG;

  TypeExtensionManager(const QString& extensionPoint);
  ~TypeExtensionManager();

  Property::Pointer GetProperty(Object::ConstPointer receiver,
                                const QString& namespaze, const QString& method);

  /*synchronized*/Property::Pointer GetProperty(Object::ConstPointer receiver,
      const QString& namespaze, const QString& method, bool forcePluginActivation);

protected:

  friend class TypeExtension;

  /*
   * This method doesn't need to be synchronized since it is called
   * from withing the getProperty method which is synchronized
   */
  TypeExtension::Pointer Get(const Reflection::TypeInfo& typeInfo);

  /*
   * This method doesn't need to be synchronized since it is called
   * from withing the getProperty method which is synchronized
   */
  QList<IPropertyTester::Pointer> LoadTesters(const QString& typeName);

private:

  /*synchronized*/void InitializeCaches();

  virtual void Added(const QList<SmartPointer<IExtension> >& extensions);
  virtual void Removed(const QList<SmartPointer<IExtension> >& extensions);
  virtual void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints);
  virtual void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints);
};

}

#endif /*BERRYTYPEEXTENSIONMANAGER_H_*/
