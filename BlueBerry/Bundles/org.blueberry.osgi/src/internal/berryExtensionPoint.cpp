/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "berryExtensionPoint.h"

#include "../service/berryIConfigurationElement.h"

#include "../berryPlatformException.h"

namespace berry {

ExtensionPoint::ExtensionPoint(const std::string& contributor)
 : m_Contributor(contributor), m_Label(""), m_SimpleId("")
{
  m_UniqueId = contributor + "." + m_SimpleId;
}

std::string
ExtensionPoint::GetContributor() const
{
  return m_Contributor;
}

const std::vector<IConfigurationElement::Pointer> ExtensionPoint::GetConfigurationElements() const
{
  std::vector<IConfigurationElement::Pointer> result;

  const std::vector<const IExtension*> extensions = this->GetExtensions();
  for (std::vector<const IExtension*>::const_iterator itr = extensions.begin();
       itr != extensions.end(); ++itr)
  {
    const std::vector<IConfigurationElement::Pointer> configs = (*itr)->GetConfigurationElements();
    result.insert(result.end(), configs.begin(), configs.end());
  }

  return result;
}

const IExtension*
ExtensionPoint::GetExtension(const std::string& extensionId) const
{
  std::map<std::string, Extension::Pointer>::const_iterator iter = m_Extensions.find(extensionId);

  if (iter == m_Extensions.end())
    return 0;

  return iter->second.GetPointer();
}

const std::vector<const IExtension*>
ExtensionPoint::GetExtensions() const
{
  std::vector<const IExtension*> extensions;
  for (std::map<std::string, Extension::Pointer>::const_iterator iter = m_Extensions.begin();
       iter != m_Extensions.end(); ++iter)
  {
    extensions.push_back(iter->second.GetPointer());
  }

  for (std::vector<Extension::Pointer>::const_iterator iter = m_UnnamedExtensions.begin();
       iter != m_UnnamedExtensions.end(); ++iter)
  {
    extensions.push_back(iter->GetPointer());
  }
  //extensions.insert(extensions.end(), m_UnnamedExtensions.begin(), m_UnnamedExtensions.end());

  return extensions;
}

std::string
ExtensionPoint::GetLabel() const
{
  return m_Label;
}

std::string
ExtensionPoint::GetSimpleIdentifier() const
{
  return m_SimpleId;
}

std::string
ExtensionPoint::GetUniqueIdentifier() const
{
  return m_UniqueId;
}

void
ExtensionPoint::SetLabel(const std::string& label)
{
  m_Label = label;
}

void
ExtensionPoint::SetSimpleId(const std::string& id)
{
  m_SimpleId = id;
}

void
ExtensionPoint::SetParentId(const std::string& id)
{
  m_UniqueId = id + "." + m_SimpleId;
}

void
ExtensionPoint::AddExtension(Extension::Pointer extension)
{
  if (extension->GetUniqueIdentifier() == "")
  {
    m_UnnamedExtensions.push_back(extension);
    return;
  }

  if (m_Extensions.find(extension->GetUniqueIdentifier()) != m_Extensions.end())
  {
    throw PlatformException("Duplicate extension id \"" + extension->GetUniqueIdentifier()
        + "\" found for extension point \"" + this->GetUniqueIdentifier() + "\" from plugin \""
        + this->GetContributor() + "\"");
  }

  m_Extensions[extension->GetUniqueIdentifier()] = extension;
}

}
