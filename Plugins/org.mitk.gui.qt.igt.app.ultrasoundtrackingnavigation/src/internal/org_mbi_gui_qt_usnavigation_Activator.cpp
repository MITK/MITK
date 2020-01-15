/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mbi_gui_qt_usnavigation_Activator.h"

#include <usModuleInitialization.h>

#include "QmitkUltrasoundCalibration.h"
#include "QmitkUSNavigationMarkerPlacement.h"
#include "QmitkUSNavigationPerspective.h"

#include "mitkVirtualTrackingDevice.h"

namespace mitk {
  ctkPluginContext* org_mbi_gui_qt_usnavigation_Activator::m_Context = nullptr;

  void org_mbi_gui_qt_usnavigation_Activator::start(ctkPluginContext* context)
  {
    m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkUltrasoundCalibration, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkUSNavigationMarkerPlacement, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkUSNavigationPerspective, context)

    // create a combined modality persistence object for loading and storing
    // combined modality objects persistently
    m_USCombinedModalityPersistence = mitk::USNavigationCombinedModalityPersistence::New();
  }

  void org_mbi_gui_qt_usnavigation_Activator::stop(ctkPluginContext*)
  {
    m_USCombinedModalityPersistence = nullptr;
    m_Context = nullptr;
  }

  ctkPluginContext *org_mbi_gui_qt_usnavigation_Activator::GetContext()
  {
    return m_Context;
  }
}

// necessary for us::GetModuleContext() in USNavigationCombinedModalityPersistence
// (see: https://www.mail-archive.com/mitk-users@lists.sourceforge.net/msg04421.html)
US_INITIALIZE_MODULE
