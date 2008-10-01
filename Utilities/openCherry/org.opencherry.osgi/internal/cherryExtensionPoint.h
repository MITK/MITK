/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYEXTENSIONPOINT_H_
#define CHERRYEXTENSIONPOINT_H_

#include "../cherryMacros.h"

#include "../service/cherryIExtensionPoint.h"
#include "../service/cherryIExtension.h"

#include "cherryExtension.h"

#include <map>

namespace cherry {

struct IConfigurationElement;

class ExtensionPoint : public IExtensionPoint, public Object
{

public:

  cherryClassMacro(ExtensionPoint);

  ExtensionPoint(const std::string& contributor);

  std::string GetContributor() const;

  const std::vector<IConfigurationElement::Pointer > GetConfigurationElements() const;

  const IExtension* GetExtension(const std::string& extensionId) const;
  const std::vector<const IExtension*> GetExtensions() const;

  std::string GetLabel() const;
  std::string GetSimpleIdentifier() const;
  std::string GetUniqueIdentifier() const;

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

#endif /*CHERRYEXTENSIONPOINT_H_*/
