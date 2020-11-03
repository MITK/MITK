/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSIONELEMENT_H_
#define BERRYEXTENSIONELEMENT_H_

#include "berryRegistryObject.h"

namespace berry {

struct IContributor;

/**
 * An object which represents the user-defined contents of an extension
 * in a plug-in manifest.
 */
class ConfigurationElement : public RegistryObject
{

private:

  //The id of the parent element. It can be a configuration element or an extension
  int parentId;
  short parentType; //This value is only interesting when running from cache.

  //Store the properties and the value of the configuration element.
  //The format is the following:
  //  [p1, v1, p2, v2, configurationElementValue]
  //If the array size is even, there is no "configurationElementValue (ie getValue returns null)".
  //The properties and their values are alternated (v1 is the value of p1).
  QList<QString> propertiesAndValue;

  //The name of the configuration element
  QString name;

  //ID of the actual contributor of this element
  //This value can be null when the element is loaded from disk and the owner has been uninstalled.
  //This happens when the configuration is obtained from a delta containing removed extension.
  QString contributorId;

protected:

  friend class ConfigurationElementHandle;
  friend class ExtensionRegistry;
  friend class ExtensionsParser;

  void ThrowException(const QString& message, const ctkException& exc);

  void ThrowException(const QString& message);

  QString GetValue() const;

  QString GetValueAsIs() const;

  QString GetAttributeAsIs(const QString& attrName) const;

  QList<QString> GetAttributeNames() const;

  void SetProperties(const QList<QString>& value);

  QList<QString> GetPropertiesAndValue() const;

  void SetValue(const QString& value);

  void SetContributorId(const QString& id);

  QString GetContributorId() const;

  void SetParentId(int objectId);

  QString GetName() const;

  void SetName(const QString& name);

  void SetParentType(short type);

  QObject* CreateExecutableExtension(const QString& attributeName);

  QString GetAttribute(const QString& attrName, const QLocale& locale) const;

  QString GetValue(const QLocale& locale) const;

public:

  berryObjectMacro(berry::ConfigurationElement);

  ConfigurationElement(ExtensionRegistry* registry, bool persist);

  ConfigurationElement(int self, const QString& contributorId,
                       const QString& name, const QList<QString>& propertiesAndValue,
                       const QList<int>& children, int extraDataOffset, int parent,
                       short parentType, ExtensionRegistry* registry, bool persist);

  QString GetAttribute(const QString& attrName) const;

  QList<ConfigurationElement::Pointer> GetChildren(const QString& childrenName) const;

  SmartPointer<IContributor> GetContributor() const;

};

}  // namespace berry


#endif /*BERRYEXTENSIONELEMENT_H_*/
