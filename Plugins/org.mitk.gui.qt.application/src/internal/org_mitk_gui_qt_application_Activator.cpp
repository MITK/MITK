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

namespace mitk
{

  org_mitk_gui_qt_application_Activator* org_mitk_gui_qt_application_Activator::m_Instance = nullptr;
  ctkPluginContext* org_mitk_gui_qt_application_Activator::m_Context = nullptr;

  void org_mitk_gui_qt_application_Activator::start(ctkPluginContext* context)
  {
    this->m_Instance = this;
    this->m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkGeneralPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkEditorsPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkShowPreferencePageHandler, context)

    QmitkRegisterClasses();

    this->m_PrefServiceTracker.reset(new ctkServiceTracker<berry::IPreferencesService*>(context));
    this->m_PrefServiceTracker->open();
  }

  void org_mitk_gui_qt_application_Activator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    this->m_PrefServiceTracker.reset();

    this->m_Context = nullptr;
    this->m_Instance = nullptr;
  }

  ctkPluginContext* org_mitk_gui_qt_application_Activator::GetContext()
  {
    return m_Context;
  }

  org_mitk_gui_qt_application_Activator *org_mitk_gui_qt_application_Activator::GetInstance()
  {
    return m_Instance;
  }

  berry::IPreferencesService* org_mitk_gui_qt_application_Activator::GetPreferencesService()
  {
    return m_PrefServiceTracker->getService();
  }

}
