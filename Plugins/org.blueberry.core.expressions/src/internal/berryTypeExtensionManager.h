/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  class nullptr_PROPERTY_TESTER_ : public IPropertyTester
  {
  public:
    bool Handles(const QString&  /*namespaze*/, const QString&  /*property*/) override
    {
      return false;
    }
    bool IsInstantiated() override
    {
      return true;
    }
    bool IsDeclaringPluginActive() override
    {
      return true;
    }
    IPropertyTester* Instantiate() override
    {
      return this;
    }
    bool Test(Object::ConstPointer, const QString& /*property*/,
              const QList<Object::Pointer>&  /*args*/, Object::Pointer  /*expectedValue*/) override
    {
      return false;
    }
  };

  static const nullptr_PROPERTY_TESTER_ nullptr_PROPERTY_TESTER;

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
  ~TypeExtensionManager() override;

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

  void Added(const QList<SmartPointer<IExtension> >& extensions) override;
  void Removed(const QList<SmartPointer<IExtension> >& extensions) override;
  void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;
  void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;
};

}

#endif /*BERRYTYPEEXTENSIONMANAGER_H_*/
