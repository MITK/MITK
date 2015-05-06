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

  QString GetNamespaceIdentifier() const;

  SmartPointer<IContributor> GetContributor() const;

  QString GetExtensionPointUniqueIdentifier() const;

  QString GetLabel() const;

  QString GetLabelAsIs() const;

  QString GetLabel(const QLocale& locale) const;

  QString GetSimpleIdentifier() const;

  QString GetUniqueIdentifier() const;

  QList<SmartPointer<IConfigurationElement> > GetConfigurationElements() const;

  bool IsValid() const;

  SmartPointer<RegistryObject> GetObject() const;

protected:

  SmartPointer<Extension> GetExtension() const;

  bool ShouldPersist() const;

  QString GetContributorId() const;

};

}

#endif // BERRYEXTENSIONHANDLE_H
