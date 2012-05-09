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

#ifndef BERRYEXTENSIONPOINT_H_
#define BERRYEXTENSIONPOINT_H_

#include "berryMacros.h"

#include <berryIExtensionPoint.h>
#include <berryIExtension.h>

#include "berryExtension.h"

#include <map>

namespace berry {

struct IConfigurationElement;

class ExtensionPoint : public IExtensionPoint
{

public:

  berryObjectMacro(ExtensionPoint)

  ExtensionPoint(const std::string& contributor);

  QString GetContributor() const;

  QList<IConfigurationElement::Pointer > GetConfigurationElements() const;

  SmartPointer<IExtension> GetExtension(const std::string& extensionId) const;
  QList<SmartPointer<IExtension> > GetExtensions() const;

  QString GetLabel() const;
  QString GetSimpleIdentifier() const;
  QString GetUniqueIdentifier() const;

  bool IsValid() const;

  void AddExtension(Extension::Pointer extension);

  void SetLabel(const std::string& label);
  void SetSimpleId(const std::string& id);
  void SetParentId(const std::string& id);

private:
  std::string m_Contributor;
  std::string m_Label;
  std::string m_SimpleId;
  std::string m_UniqueId;

  std::map<std::string, Extension::Pointer> m_Extensions;
  std::vector<Extension::Pointer> m_UnnamedExtensions;

};

}

#endif /*BERRYEXTENSIONPOINT_H_*/
