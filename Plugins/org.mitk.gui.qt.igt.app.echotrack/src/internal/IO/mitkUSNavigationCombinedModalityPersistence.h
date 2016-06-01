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

#include "mitkCommon.h"
#include "mitkUSCombinedModality.h"
#include "mitkNavigationDataSource.h"

// Microservices
#include <usModuleContext.h>

namespace mitk {
/**
 * \brief Persistence for the mitk::CombinedModality micro services.
 * The micro services are saved into QSettings on destruction of this objects
 * and they are loaded again when their NavigationDataSource and USDevice will
 * become registered as a micro service.
 *
 * The NavigationDataSources are identified by their name and the USDevices are
 * identified by their manufacturer, model and comment.
 *
 * For using this persistence class for combined modalites an object of this
 * class has to be created an held as long as newly created combined modalities
 * should be made persistent.
 */
class USNavigationCombinedModalityPersistence : public itk::Object
{
public:
  mitkClassMacroItkParent(USNavigationCombinedModalityPersistence, itk::Object)
  itkNewMacro(Self)
protected:
  /**
   * \brief Restores combined modality from QSettings and registers on service events.
   * Combined modalities are registered as a micro service as soon as their
   * NavigationDataSource and USDevice are available. The constructor adds
   * mitk::USNavigationCombinedModalityPersistence::OnServiceEvent() as a
   * listener to the module context to be notified when micro services become
   * available.
   */
  USNavigationCombinedModalityPersistence();

  /**
    * \brief The destructor stores combined modalities by calling mitk::USNavigationCombinedModalityPersistence::StoreCurrentDevices().
    */
  ~USNavigationCombinedModalityPersistence();
private:
  void OnServiceEvent(const us::ServiceEvent event);

  void StoreCurrentDevices();
  void LoadStoredDevices();

  /**
   * \brief Try to get combined modality with given properties from the service registry.
   * \return the combined modality if it was found, null otherwise
   */
  mitk::USCombinedModality::Pointer GetCombinedModality(std::string manufacturer, std::string model);

  /**
   * \brief Try to get ultrasound device with given properties from the service registry.
   * \return the ultrasound device if it was found, null otherwise
   */
  mitk::USDevice::Pointer GetUSDevice(std::string manufacturer, std::string model, std::string comment);

  /**
   * \brief Try to get navigation data source with given properties from the service registry.
   * \return the navigation data source if it was found, null otherwise
   */
  mitk::NavigationDataSource::Pointer GetNavigationDataSource(std::string name);
};
} // namespace mitk

#endif // USCOMBINEDMODALITYPERSISTENCE_H
