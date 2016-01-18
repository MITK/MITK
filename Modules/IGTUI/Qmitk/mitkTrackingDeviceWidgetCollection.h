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

#ifndef TRACKINGDEVICEWidgetCollection_H_INCLUDED
#define TRACKINGDEVICEWidgetCollection_H_INCLUDED

#include <MitkIGTUIExports.h>

#include "mitkTrackingTypes.h"
#include "mitkTrackingDeviceTypeInformation.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

#include "QmitkAbstractTrackingDeviceWidget.h"

namespace mitk {
  /** Documentation:
  *   \brief This class is a collection for all TrackingDeviceWidgets (derived from AbstractTrackingDeviceWidget)
  *   The Collection is avaiable via Microservice.
  *   If you want to add your own tracking device to the Tracking Toolbox, you should register your widget configuring your device
  *   to this collection using the RegisterTrackingDeviceWidget function.
  *   \ingroup IGTUI
  */
  class MITKIGTUI_EXPORT TrackingDeviceWidgetCollection
  {
  public:

    TrackingDeviceWidgetCollection();
    ~TrackingDeviceWidgetCollection();

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    * To unregister, call UnregisterMicroservice().
    */
    virtual void RegisterAsMicroservice();

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    */
    virtual void UnRegisterMicroservice();

    void RegisterTrackingDeviceWidget(TrackingDeviceType type, QmitkAbstractTrackingDeviceWidget* widget);

    QmitkAbstractTrackingDeviceWidget* GetTrackingDeviceWidgetClone(TrackingDeviceType type);

  private:

    std::string m_Name;

    us::ServiceRegistration<TrackingDeviceWidgetCollection> m_ServiceRegistration;

    std::vector<std::pair <TrackingDeviceType, QmitkAbstractTrackingDeviceWidget*>> m_TrackingDeviceWidgets;
  };
} // namespace mitk
MITK_DECLARE_SERVICE_INTERFACE(mitk::TrackingDeviceWidgetCollection, "org.mitk.services.TrackingDeviceWidgetCollection")
#endif //TRACKINGDEVICEWidgetCollection_H_INCLUDED
