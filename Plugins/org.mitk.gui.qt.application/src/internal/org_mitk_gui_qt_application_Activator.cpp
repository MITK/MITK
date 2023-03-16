/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_application_Activator.h"

#include "QmitkGeneralPreferencePage.h"
#include "QmitkEditorsPreferencePage.h"

#include <QmitkRegisterClasses.h>

#include "QmitkShowPreferencePageHandler.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  ctkPluginContext* org_mitk_gui_qt_application_Activator::m_Context = nullptr;

  void org_mitk_gui_qt_application_Activator::start(ctkPluginContext* context)
  {
    m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkGeneralPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkEditorsPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkShowPreferencePageHandler, context)

    QmitkRegisterClasses();
  }

  void org_mitk_gui_qt_application_Activator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
    m_Context = nullptr;
  }

  ctkPluginContext* org_mitk_gui_qt_application_Activator::GetContext()
  {
    return m_Context;
  }

}
