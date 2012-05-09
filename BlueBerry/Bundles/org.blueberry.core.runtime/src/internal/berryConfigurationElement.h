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

#ifndef BERRYEXTENSIONELEMENT_H_
#define BERRYEXTENSIONELEMENT_H_

#include "berryMacros.h"

#include "berryBundleLoader.h"

#include "Poco/DOM/Node.h"

#include <berryIConfigurationElement.h>

namespace berry {

class Extension;

class ConfigurationElement : public IConfigurationElement
{

public:

  berryObjectMacro(ConfigurationElement);

  ConfigurationElement(BundleLoader* loader, Poco::XML::Node* config,
                     std::string contributor, SmartPointer<Extension> extension,
                     const ConfigurationElement* parent = 0);

  QObject* CreateExecutableExtension(const QString& propertyName) const;

  QString GetAttribute(const QString& name) const;
  QStringList GetAttributeNames() const;

  bool GetBoolAttribute(const std::string& name, bool& value) const;

  QList<IConfigurationElement::Pointer> GetChildren() const;
  QList<IConfigurationElement::Pointer> GetChildren(const QString& name) const;

  QString GetValue() const;

  QString GetNamespaceIdentifier() const;

  QString GetName() const;
  const IConfigurationElement* GetParent() const;

  QString GetContributor() const;
  const IExtension* GetDeclaringExtension() const;

  bool IsValid() const;

  ~ConfigurationElement();


private:
  Poco::XML::Node* m_ConfigurationNode;

  const ConfigurationElement* m_Parent;
  SmartPointer<Extension> m_Extension;

};

}  // namespace berry


#endif /*BERRYEXTENSIONELEMENT_H_*/
