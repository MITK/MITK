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

#include "org_mitk_gui_qt_application_Activator.h"

#include "QmitkGeneralPreferencePage.h"
#include "QmitkEditorsPreferencePage.h"

#include <mitkGlobalInteraction.h>
#include <QmitkRegisterClasses.h>

#include <QtPlugin>

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include "mitkModuleResourceStream.h"
#include "mitkModuleRegistry.h"

namespace mitk
{

  ctkPluginContext* org_mitk_gui_qt_application_Activator::m_Context = 0;

  void org_mitk_gui_qt_application_Activator::start(ctkPluginContext* context)
  {
    this->m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkGeneralPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkEditorsPreferencePage, context)

    Module* module = ModuleRegistry::GetModule("Mitk");
    ModuleResource resource = module->GetResource("Interactions/Legacy/QtStateMachine.xml");
    if (!resource.IsValid())
    {
      mitkThrow()<< ("Resource not valid. State machine pattern not found:Interactions/Legacy/QtStateMachine.xml" );
    }
    mitk::ModuleResourceStream stream(resource);

     std::string patternString((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
     mitk::GlobalInteraction::GetInstance()->Initialize("global", patternString);

    QmitkRegisterClasses();
  }

  void org_mitk_gui_qt_application_Activator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    this->m_Context = 0;
  }

  ctkPluginContext* org_mitk_gui_qt_application_Activator::GetContext()
  {
    return m_Context;
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_application, mitk::org_mitk_gui_qt_application_Activator)

