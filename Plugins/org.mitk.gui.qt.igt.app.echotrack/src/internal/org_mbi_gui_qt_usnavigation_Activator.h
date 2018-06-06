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
    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);

    static ctkPluginContext* GetContext();

  private:
    static ctkPluginContext* m_Context;

    TrackingDeviceSource::Pointer m_VirtualTrackingDeviceSource;
    USNavigationCombinedModalityPersistence::Pointer m_USCombinedModalityPersistence;
  };

  typedef org_mbi_gui_qt_usnavigation_Activator PluginActivator;
}

#endif
