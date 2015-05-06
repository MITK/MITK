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

#ifndef BERRYREGISTRYOBJECTFACTORY_H
#define BERRYREGISTRYOBJECTFACTORY_H

#include <QList>

namespace berry {

template<class T> class SmartPointer;

class ConfigurationElement;
class Extension;
class ExtensionPoint;
class ExtensionRegistry;
class RegistryContribution;

/**
 * A factory method for the creation of the registry objects.
 */
class RegistryObjectFactory
{

protected:

  // The extension registry that this element factory works in
  ExtensionRegistry* registry;

public:

  RegistryObjectFactory(ExtensionRegistry* registry);

  virtual ~RegistryObjectFactory();

  ////////////////////////////////////////////////////////////////////////////
  // Contribution
  virtual SmartPointer<RegistryContribution> CreateContribution(const QString& contributorId,
                                                                bool persist);

  ////////////////////////////////////////////////////////////////////////////
  // Extension point
  virtual SmartPointer<ExtensionPoint> CreateExtensionPoint(bool persist);

  virtual SmartPointer<ExtensionPoint> CreateExtensionPoint(int self, const QList<int> children,
                                                            int dataOffset, bool persist);

  ////////////////////////////////////////////////////////////////////////////
  // Extension
  virtual SmartPointer<Extension> CreateExtension(bool persist);

  virtual SmartPointer<Extension> CreateExtension(int self, const QString& simpleId,
                                                  const QString& namespaze, const QList<int>& children,
                                                  int extraData, bool persist);

  ////////////////////////////////////////////////////////////////////////////
  // Configuration element
  virtual SmartPointer<ConfigurationElement> CreateConfigurationElement(bool persist);

  virtual SmartPointer<ConfigurationElement> CreateConfigurationElement(int self, const QString& contributorId,
                                                                        const QString& name,
                                                                        const QList<QString>& propertiesAndValue,
                                                                        const QList<int>& children,
                                                                        int extraDataOffset, int parent,
                                                                        short parentType, bool persist);
};

}

#endif // BERRYREGISTRYOBJECTFACTORY_H
