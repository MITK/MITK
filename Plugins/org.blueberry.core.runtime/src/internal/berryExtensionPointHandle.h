/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(berry::ExtensionPointHandle);

  ExtensionPointHandle(const SmartPointer<const IObjectManager>& objectManager, int id);
  ExtensionPointHandle(const IObjectManager* objectManager, int id);

  QList<SmartPointer<IExtension> > GetExtensions() const override;

  QString GetNamespaceIdentifier() const override;

  SmartPointer<IContributor> GetContributor() const override;

  SmartPointer<IExtension> GetExtension(const QString& extensionId) const override;

  QList<SmartPointer<IConfigurationElement> > GetConfigurationElements() const override;

  QString GetLabelAsIs() const;

  QString GetLabel() const override;

  QString GetLabel(const QLocale& locale) const;

  QString GetSchemaReference() const;

  QString GetSimpleIdentifier() const override;

  QString GetUniqueIdentifier() const override;

  SmartPointer<RegistryObject> GetObject() const override;

  bool IsValid() const override;

protected:

  bool ShouldPersist() const;

  SmartPointer<ExtensionPoint> GetExtensionPoint() const;
};

}

#endif // BERRYEXTENSIONPOINTHANDLE_H
