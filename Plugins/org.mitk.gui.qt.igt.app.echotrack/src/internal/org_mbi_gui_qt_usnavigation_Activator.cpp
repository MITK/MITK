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

#include "QmitkUltrasoundCalibration.h"
#include "QmitkUSNavigationMarkerPlacement.h"
#include "QmitkUSNavigationPerspective.h"

#include "mitkVirtualTrackingDevice.h"

namespace mitk {
  ctkPluginContext* org_mbi_gui_qt_usnavigation_Activator::m_Context = 0;

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
    m_USCombinedModalityPersistence = 0;
    m_Context = 0;
  }

  ctkPluginContext *org_mbi_gui_qt_usnavigation_Activator::GetContext()
  {
    return m_Context;
  }
}

// necessary for us::GetModuleContext() in USNavigationCombinedModalityPersistence
// (see: https://www.mail-archive.com/mitk-users@lists.sourceforge.net/msg04421.html)
US_INITIALIZE_MODULE
