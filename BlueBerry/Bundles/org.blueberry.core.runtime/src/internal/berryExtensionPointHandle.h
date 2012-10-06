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

#ifndef BERRYEXTENSIONPOINTHANDLE_H
#define BERRYEXTENSIONPOINTHANDLE_H

#include "berryHandle.h"
#include "berryIExtensionPoint.h"

namespace berry {

struct IObjectManager;

class ExtensionPoint;

class ExtensionPointHandle : public Handle, public IExtensionPoint
{
public:

  berryObjectMacro(berry::ExtensionPointHandle)

  ExtensionPointHandle(const SmartPointer<const IObjectManager>& objectManager, int id);
  ExtensionPointHandle(const IObjectManager* objectManager, int id);

  QList<SmartPointer<IExtension> > GetExtensions() const;

  QString GetNamespaceIdentifier() const;

  SmartPointer<IContributor> GetContributor() const;

  SmartPointer<IExtension> GetExtension(const QString& extensionId) const;

  QList<SmartPointer<IConfigurationElement> > GetConfigurationElements() const;

  QString GetLabelAsIs() const;

  QString GetLabel() const;

  QString GetLabel(const QLocale& locale) const;

  QString GetSchemaReference() const;

  QString GetSimpleIdentifier() const;

  QString GetUniqueIdentifier() const;

  SmartPointer<RegistryObject> GetObject() const;

  bool IsValid() const;

protected:

  bool ShouldPersist() const;

  SmartPointer<ExtensionPoint> GetExtensionPoint() const;
};

}

#endif // BERRYEXTENSIONPOINTHANDLE_H
