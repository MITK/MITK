/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef org_mbi_gui_qt_usnavigation_Activator_h
#define org_mbi_gui_qt_usnavigation_Activator_h

#include <ctkPluginActivator.h>
#include "mitkTrackingDeviceSource.h"
#include "IO/mitkUSNavigationCombinedModalityPersistence.h"

namespace mitk
{
  class org_mbi_gui_qt_usnavigation_Activator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org_mbi_gui_qt_usnavigation")
    Q_INTERFACES(ctkPluginActivator)

  public:
    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

    static ctkPluginContext* GetContext();

  private:
    static ctkPluginContext* m_Context;

    TrackingDeviceSource::Pointer m_VirtualTrackingDeviceSource;
    USNavigationCombinedModalityPersistence::Pointer m_USCombinedModalityPersistence;
  };

  typedef org_mbi_gui_qt_usnavigation_Activator PluginActivator;
}

#endif
