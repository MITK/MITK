/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef TrackingDeviceWidgetCollection_H_INCLUDED
#define TrackingDeviceWidgetCollection_H_INCLUDED

#include <MitkIGTUIExports.h>

#include "mitkTrackingDeviceTypeInformation.h"
#include "QmitkAbstractTrackingDeviceWidget.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>


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

    us::ServiceRegistration<TrackingDeviceWidgetCollection> m_ServiceRegistration;

    std::vector<std::pair<TrackingDeviceType, QmitkAbstractTrackingDeviceWidget*> > m_TrackingDeviceWidgets;
  };
} // namespace mitk
MITK_DECLARE_SERVICE_INTERFACE(mitk::TrackingDeviceWidgetCollection, "org.mitk.services.TrackingDeviceWidgetCollection")
#endif //TrackingDeviceWidgetCollection_H_INCLUDED
