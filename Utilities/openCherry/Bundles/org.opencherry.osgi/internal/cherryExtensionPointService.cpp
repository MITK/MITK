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

#include "cherryExtensionPointService.h"
#include "cherryConfigurationElement.h"
#include "cherryExtensionPoint.h"

#include "Poco/Exception.h"

#include "cherryBundle.h"

#include "Poco/StringTokenizer.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/NamedNodeMap.h"

#include <iostream>

namespace cherry {

bool 
ExtensionPointService::IsA(const std::type_info& type)
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info& 
ExtensionPointService::GetType() const
{
  return typeid(IExtensionPointService);
}

ExtensionPointService::ExtensionPointService(BundleLoader* loader) 
  : m_BundleLoader(loader)
{
  
}

void 
ExtensionPointService::AddContribution(std::istream& istr, 
                                            const std::string& contributor)
{
  m_XMLInputSource.setByteStream(istr);
  Poco::XML::Document* document = m_DOMParser.parse(&m_XMLInputSource);
  
  Poco::XML::NodeList* nodes = document->getElementsByTagName("extension-point");
  for (unsigned long i = 0; i < nodes->length(); ++i)
  {
    Poco::XML::NamedNodeMap* attributes = nodes->item(i)->attributes();
    Poco::XML::Node* attr = attributes->getNamedItem("id");
    std::string simpleId = attr->nodeValue();
    attr = attributes->getNamedItem("name");
    std::string label = "";
    if (attr != 0) label = attr->nodeValue();
    
    ExtensionPoint::Pointer xp(new ExtensionPoint(contributor));
    xp->SetLabel(label);
    Poco::StringTokenizer tokenizer(simpleId, ".");
    if (tokenizer.count() == 1)
    {
      xp->SetSimpleId(simpleId);
      xp->SetParentId(contributor);
    }
    else
    {
      std::string parentId = "";
      for (std::size_t i = 0; i < tokenizer.count()-1; ++i)
      {
        if (i > 0) parentId += ".";
        parentId += tokenizer[i];
      }
      xp->SetSimpleId(tokenizer[tokenizer.count()-1]);
      xp->SetParentId(parentId);
    }
    
    m_ExtensionPointMap[xp->GetUniqueIdentifier()] = xp;
    
    std::cout << "Extension-Point found: " << xp->GetUniqueIdentifier() << " (from " << xp->GetContributor() << ")\n";
    
    attributes->release();
  }
  nodes->release();
  
  nodes = document->getElementsByTagName("extension");
  for (unsigned long i = 0; i < nodes->length(); ++i)
  {
    Poco::XML::NamedNodeMap* attributes = nodes->item(i)->attributes();
    Poco::XML::Node* attr = attributes->getNamedItem("point");
    if (attr == 0) continue;
    
    std::string xp = attr->nodeValue();
    std::cout << "Extension found for extension-point: " << xp << std::endl;
    if (m_ExtensionPointMap[xp].IsNull())
    {
      std::cout << "Extension-point unknown, extension skipped.\n";
      continue;
    }
    
    Extension::Pointer extension(new Extension(contributor));
    extension->SetExtensionPointIdentifier(xp);
    attr = attributes->getNamedItem("id");
    if (attr)
      extension->SetSimpleIdentifier(attr->nodeValue());
    
    std::vector<IConfigurationElement::Pointer> children;
    if (nodes->item(i)->hasChildNodes())
    {
      Poco::XML::NodeList* ch = nodes->item(i)->childNodes();
      for (unsigned long childIndex = 0; childIndex < ch->length(); ++childIndex)
      {
        Poco::XML::Node* child = ch->item(childIndex);
        if (dynamic_cast<Poco::XML::Element*>(child) == 0) continue;
        IConfigurationElement::Pointer elem(
            new ConfigurationElement(m_BundleLoader, 
                                     child, 
                                     contributor, extension, 0));
        children.push_back(elem);
      }
      ch->release();
    }
    
    extension->SetSubElements(children);
    
    m_ExtensionPointMap[xp]->AddExtension(extension);

    attributes->release();
  }
  //nodes->release();
  
  m_Contributors.insert(contributor);
}

bool
ExtensionPointService::HasContributionFrom(const std::string& name) const
{
  return m_Contributors.find(name) != m_Contributors.end();
}
  
const std::vector<IConfigurationElement::Pointer>
ExtensionPointService::GetConfigurationElementsFor(const std::string& extensionPointId) const
{
  std::cout << "Getting configuration elements for point: " << extensionPointId << std::endl;
  
  std::vector<IConfigurationElement::Pointer> configs;
  const IExtensionPoint* xp = this->GetExtensionPoint(extensionPointId);
  if (xp != 0)
  {
    std::vector<const IExtension*> extensions(xp->GetExtensions());
    for (std::vector<const IExtension*>::const_iterator extension = extensions.begin();
         extension != extensions.end(); ++extension)
    {
      std::vector<IConfigurationElement::Pointer> c((*extension)->GetConfigurationElements());
      configs.insert(configs.end(), c.begin(), c.end());
    }
  }
  
  return configs;
}
  
const IExtensionPoint*
ExtensionPointService::GetExtensionPoint(const std::string& id) const
{
  ExtensionPointMap::const_iterator iter;
  iter = m_ExtensionPointMap.find(id);
  if (iter != m_ExtensionPointMap.end()) return iter->second;
  return 0;
}

const IExtension* 
ExtensionPointService::GetExtension(const std::string& extensionPointId, const std::string& extensionId) const
{
  const IExtensionPoint* xp = this->GetExtensionPoint(extensionPointId);
  if (xp != 0)
  {
    return xp->GetExtension(extensionId);
  }
  
  return 0;
}

const std::vector<const IExtension*> 
ExtensionPointService::GetExtensions(const std::string& contributor) const
{
  std::vector<const IExtension*> extensions;
  for (std::map<std::string,ExtensionPoint::Pointer>::const_iterator iter = m_ExtensionPointMap.begin();
       iter != m_ExtensionPointMap.end(); ++iter)
  {
    const std::vector<const IExtension*> ext(iter->second->GetExtensions());
    for (std::vector<const IExtension*>::const_iterator extension = ext.begin();
         extension != ext.end(); ++extension)
    {
      if ((*extension)->GetNamespace() == contributor)
        extensions.push_back(*extension);
    }
  }
  
  return extensions;
}
  
const std::vector<const IExtensionPoint*> 
ExtensionPointService::GetExtensionPoints() const
{
  std::vector<const IExtensionPoint*> xps;
  for (std::map<std::string,ExtensionPoint::Pointer>::const_iterator iter = m_ExtensionPointMap.begin();
       iter != m_ExtensionPointMap.end(); ++iter)
  {
    xps.push_back(iter->second);
  }
  
  return xps;
}
  
const std::vector<const IExtensionPoint*> 
ExtensionPointService::GetExtensionPoints(const std::string& contributor) const
{
  std::vector<const IExtensionPoint*> xps;
  for (std::map<std::string,ExtensionPoint::Pointer>::const_iterator iter = m_ExtensionPointMap.begin();
       iter != m_ExtensionPointMap.end(); ++iter)
  {
    if (iter->second->GetContributor() == contributor)
      xps.push_back(iter->second);
  }
  
  return xps;
}


}
