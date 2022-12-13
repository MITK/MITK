/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_dosevisualization_Activator.h"
#include "RTDoseVisualizer.h"
#include "RTUIPreferencePage.h"
#include "DoseVisualizationPreferencePage.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {

  ctkPluginContext* org_mitk_gui_qt_dosevisualization_Activator::m_Context = nullptr;

  void org_mitk_gui_qt_dosevisualization_Activator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(RTDoseVisualizer, context);
    BERRY_REGISTER_EXTENSION_CLASS(RTUIPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(DoseVisualizationPreferencePage, context)

      m_Context = context;
  }

  void org_mitk_gui_qt_dosevisualization_Activator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context);

    m_Context = nullptr;
  }

  ctkPluginContext* org_mitk_gui_qt_dosevisualization_Activator::GetContext()
  {
    return m_Context;
  }

}
