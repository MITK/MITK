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

  berryObjectMacro(berry::ExtensionHandle)

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
