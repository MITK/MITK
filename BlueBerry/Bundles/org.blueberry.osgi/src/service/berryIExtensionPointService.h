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

#ifndef BERRYIEXTENSIONPOINTSERVICE_H_
#define BERRYIEXTENSIONPOINTSERVICE_H_

#include <org_blueberry_osgi_Export.h>

#include "berryService.h"
#include "berryIExtensionPoint.h"
#include "berryIConfigurationElement.h"

#include <QtPlugin>

namespace berry {

struct IBundle;

struct BERRY_OSGI IExtensionPointService : public Service
{
  berryInterfaceMacro(IExtensionPointService, berry);

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

Q_DECLARE_INTERFACE(berry::IExtensionPointService, "org.blueberry.service.IExtensionPoint")

#endif /*BERRYIEXTENSIONPOINTSERVICE_H_*/
