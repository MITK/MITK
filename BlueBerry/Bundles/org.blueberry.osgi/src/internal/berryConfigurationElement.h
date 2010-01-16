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

#ifndef BERRYEXTENSIONELEMENT_H_
#define BERRYEXTENSIONELEMENT_H_

#include "../berryMacros.h"

#include "../berryBundleLoader.h"

#include "Poco/DOM/Node.h"

#include "../service/berryIConfigurationElement.h"

namespace berry {

class Extension;

class ConfigurationElement : public IConfigurationElement
{

public:

  berryObjectMacro(ConfigurationElement);

  ConfigurationElement(BundleLoader* loader, Poco::XML::Node* config,
                     std::string contributor, SmartPointer<Extension> extension,
                     const ConfigurationElement* parent = 0);

  bool GetAttribute(const std::string& name, std::string& value) const;
  bool GetBoolAttribute(const std::string& name, bool& value) const;

  const std::vector<IConfigurationElement::Pointer> GetChildren() const;
  const std::vector<IConfigurationElement::Pointer> GetChildren(const std::string& name) const;

  std::string GetValue() const;

  std::string GetName() const;
  const IConfigurationElement* GetParent() const;

  const std::string& GetContributor() const;
  const IExtension* GetDeclaringExtension() const;

  ~ConfigurationElement();


private:
  Poco::XML::Node* m_ConfigurationNode;

  const ConfigurationElement* m_Parent;
  SmartPointer<Extension> m_Extension;

};

}  // namespace berry


#endif /*BERRYEXTENSIONELEMENT_H_*/
