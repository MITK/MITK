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


#include "org_mbi_gui_qt_usnavigation_Activator.h"

#include <usModuleInitialization.h>
#include <QtPlugin>

//#include "USNavigation.h"
#include "UltrasoundCalibration.h"
#include "USNavigationMarkerPlacement.h"
#include "QmitkUSNavigationPerspective.h"

#include "mitkVirtualTrackingDevice.h"

namespace mitk {
ctkPluginContext* org_mbi_gui_qt_usnavigation_Activator::m_Context = 0;

void org_mbi_gui_qt_usnavigation_Activator::start(ctkPluginContext* context)
{
  m_Context = context;

  //BERRY_REGISTER_EXTENSION_CLASS(USNavigation, context)
  BERRY_REGISTER_EXTENSION_CLASS(UltrasoundCalibration, context)
  BERRY_REGISTER_EXTENSION_CLASS(USNavigationMarkerPlacement, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUSNavigationPerspective, context)

  // create a virtual tracking device with two tools
  mitk::VirtualTrackingDevice::Pointer device = mitk::VirtualTrackingDevice::New();
  device->AddTool("tool1");
  device->AddTool("tool2");

  m_VirtualTrackingDeviceSource = mitk::TrackingDeviceSource::New();
  m_VirtualTrackingDeviceSource->SetTrackingDevice(device);
  m_VirtualTrackingDeviceSource->Connect();
  m_VirtualTrackingDeviceSource->StartTracking();
  m_VirtualTrackingDeviceSource->RegisterAsMicroservice();

  // create a combined modality persitence object for loading and storing
  // combined modality objects persistently
  m_USCombinedModalityPersistence = mitk::USNavigationCombinedModalityPersistence::New();
}

void org_mbi_gui_qt_usnavigation_Activator::stop(ctkPluginContext* context)
{
  m_USCombinedModalityPersistence = 0;

  m_Context = 0;

  Q_UNUSED(context)
}

ctkPluginContext *org_mbi_gui_qt_usnavigation_Activator::GetContext()
{
  return m_Context;
}
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mbi_gui_qt_usnavigation, mitk::org_mbi_gui_qt_usnavigation_Activator)
#endif

// necessary for us::GetModuleContext() in USNavigationCombinedModalityPersistence
// (see: https://www.mail-archive.com/mitk-users@lists.sourceforge.net/msg04421.html)
US_INITIALIZE_MODULE
