/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef USCOMBINEDMODALITYPERSISTENCE_H
#define USCOMBINEDMODALITYPERSISTENCE_H

#include <itkObjectFactory.h>

#include <MitkUSNavigationExports.h>
#include "mitkCommon.h"
#include "mitkUSCombinedModality.h"
#include "mitkNavigationDataSource.h"

// Microservices
#include <usModuleContext.h>

namespace mitk {

class MitkUSNavigation_EXPORT USCombinedModalityPersistence : public itk::Object
{
public:
  mitkClassMacro(USCombinedModalityPersistence, itk::Object)
  itkNewMacro(Self)
protected:
  USCombinedModalityPersistence();
  ~USCombinedModalityPersistence();
private:
  void OnServiceEvent(const us::ServiceEvent event);

  void StoreCurrentDevices();
  void LoadStoredDevices();

  mitk::USCombinedModality::Pointer GetCombinedModality(std::string manufacturer, std::string model);
  mitk::USDevice::Pointer GetUSDevice(std::string manufacturer, std::string model, std::string comment);
  mitk::NavigationDataSource::Pointer GetNavigationDataSource(std::string name);
};

} // namespace mitk

#endif // USCOMBINEDMODALITYPERSISTENCE_H
