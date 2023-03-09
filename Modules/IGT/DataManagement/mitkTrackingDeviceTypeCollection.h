/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTrackingDeviceTypeCollection_h
#define mitkTrackingDeviceTypeCollection_h

#include <MitkIGTExports.h>

#include "mitkTrackingDeviceTypeInformation.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

namespace mitk {
  /** Documentation:
  *   \brief This class is a collection for information of all Tracking Device Types (derived from abstract TrackingDeviceTypeInformation)
  *   The Collection is avaiable via Microservice.
  *   If you want to add your own tracking device (e.g. to the Tracking Toolbox), you should register
  *   information about your tracking device in this collection using the RegisterTrackingDeviceType function.
  *
  *   The Microservice provides all compatible TrackingDeviceDatas for a requested tracking device type or a list of all available Tracking Devices.
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT TrackingDeviceTypeCollection
  {
  public:

    TrackingDeviceTypeCollection();
    ~TrackingDeviceTypeCollection();

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    * To unregister, call UnregisterMicroservice().
    */
    virtual void RegisterAsMicroservice();

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    */
    virtual void UnRegisterMicroservice();

    void RegisterTrackingDeviceType(TrackingDeviceTypeInformation* typeInformation);

    TrackingDeviceTypeInformation* GetTrackingDeviceTypeInformation(TrackingDeviceType type);

    std::vector<std::string> GetTrackingDeviceTypeNames();

    /**
    * /brief Returns all devices compatibel to the given Line of Devices
    */
    std::vector<TrackingDeviceData> GetDeviceDataForLine(TrackingDeviceType type);

    /**
    * /brief Returns the first TrackingDeviceData matching a given line. Useful for backward compatibility
    * with the old way to manage devices.
    */
    TrackingDeviceData GetFirstCompatibleDeviceDataForLine(TrackingDeviceType type);

    /**
    * /brief Returns the device Data set matching the model name or the invalid device, if none was found.
    */
    TrackingDeviceData GetDeviceDataByName(const std::string& modelName);

  private:

    us::ServiceRegistration<TrackingDeviceTypeCollection> m_ServiceRegistration;

    std::vector<TrackingDeviceTypeInformation*> m_TrackingDeviceTypeInformations;
  };
} // namespace mitk
MITK_DECLARE_SERVICE_INTERFACE(mitk::TrackingDeviceTypeCollection, "org.mitk.services.TrackingDeviceTypeCollection")
#endif
