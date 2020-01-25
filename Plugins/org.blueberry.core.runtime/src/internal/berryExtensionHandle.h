/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSIONHANDLE_H
#define BERRYEXTENSIONHANDLE_H

#include "berryHandle.h"
#include "berryIExtension.h"

namespace berry {

struct IObjectManager;

class Extension;

class ExtensionHandle : public Handle, public IExtension
{

public:

  berryObjectMacro(berry::ExtensionHandle);

  ExtensionHandle(const SmartPointer<const IObjectManager>& objectManager, int id);
  ExtensionHandle(const IObjectManager* objectManager, int id);

  QString GetNamespaceIdentifier() const override;

  SmartPointer<IContributor> GetContributor() const override;

  QString GetExtensionPointUniqueIdentifier() const override;

  QString GetLabel() const override;

  QString GetLabelAsIs() const;

  QString GetLabel(const QLocale& locale) const;

  QString GetSimpleIdentifier() const override;

  QString GetUniqueIdentifier() const override;

  QList<SmartPointer<IConfigurationElement> > GetConfigurationElements() const override;

  bool IsValid() const override;

  SmartPointer<RegistryObject> GetObject() const override;

protected:

  SmartPointer<Extension> GetExtension() const;

  bool ShouldPersist() const;

  QString GetContributorId() const;

};

}

#endif // BERRYEXTENSIONHANDLE_H
