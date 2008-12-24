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

#ifndef CHERRYEXTENSIONPOINTSERVICE_H_
#define CHERRYEXTENSIONPOINTSERVICE_H_

#include "../cherryMacros.h"

#include "Poco/SAX/InputSource.h"
#include "Poco/DOM/DOMParser.h"

#include "../service/cherryIConfigurationElement.h"
#include "cherryExtensionPoint.h"
#include "../service/cherryIExtensionPointService.h"

#include <set>

namespace cherry {

class Bundle;
class BundleLoader;

class ExtensionPointService : public IExtensionPointService
{
  cherryClassMacro(ExtensionPointService);
  
public:
  
  bool IsA(const std::type_info& type);
  const std::type_info& GetType() const;
  
  ExtensionPointService(BundleLoader* loader);
  
  void AddContribution(std::istream& istr, const std::string& contributor);
  
  const std::vector<IConfigurationElement::Pointer> GetConfigurationElementsFor(const std::string& extensionPointId) const;
  
  const IExtension* GetExtension(const std::string& extensionPointId, const std::string& extensionId) const;
  
  const IExtensionPoint* GetExtensionPoint(const std::string& id) const;
  
  const std::vector<const IExtension*> GetExtensions(const std::string& contributor) const;
  const std::vector<const IExtensionPoint*> GetExtensionPoints() const;
  const std::vector<const IExtensionPoint*> GetExtensionPoints(const std::string& contributor) const;
  
  bool HasContributionFrom(const std::string& name) const;
  
private:
  typedef std::map<std::string, ExtensionPoint::Pointer > ExtensionPointMap;
  
  std::set<std::string> m_Contributors;
  
  BundleLoader* m_BundleLoader;
  ExtensionPointMap m_ExtensionPointMap;
  
  Poco::XML::DOMParser m_DOMParser;
  Poco::XML::InputSource m_XMLInputSource;
};

}  // namespace cherry

#endif /*CHERRYEXTENSIONPOINTSERVICE_H_*/
