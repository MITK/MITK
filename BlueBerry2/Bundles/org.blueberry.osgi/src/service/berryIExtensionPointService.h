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

#ifndef BERRYIEXTENSIONPOINTSERVICE_H_
#define BERRYIEXTENSIONPOINTSERVICE_H_

#include "../berryOSGiDll.h"

#include "berryService.h"
#include "berryIExtensionPoint.h"
#include "berryIConfigurationElement.h"

namespace osgi {
namespace framework {
  struct IBundle;
}
}

namespace berry {

struct BERRY_OSGI IExtensionPointService : public Service
{
  osgiObjectMacro(IExtensionPointService);
  
public:
  static std::string SERVICE_ID;
  
  virtual ~IExtensionPointService() {}
  
  virtual void AddContribution(std::istream& istr, const std::string& contributor) = 0;
  
  virtual const std::vector<IConfigurationElement::Pointer> GetConfigurationElementsFor(const std::string& extensionPointId) const = 0;
  
  virtual const IExtension* GetExtension(const std::string& extensionPointId, const std::string& extensionId) const = 0;
  
  virtual const IExtensionPoint* GetExtensionPoint(const std::string& extensionPointId) const = 0;
  
  virtual const std::vector<const IExtension*> GetExtensions(const std::string& contributor) const = 0;
  virtual const std::vector<const IExtensionPoint*> GetExtensionPoints() const = 0;
  virtual const std::vector<const IExtensionPoint*> GetExtensionPoints(const std::string& contributor) const = 0;
  
  virtual bool HasContributionFrom(const std::string& name) const = 0;
  
};

}  // namespace berry

#endif /*BERRYIEXTENSIONPOINTSERVICE_H_*/
