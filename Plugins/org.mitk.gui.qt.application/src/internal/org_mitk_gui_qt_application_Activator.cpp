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

namespace mitk {

ctkPluginContext* org_mitk_gui_qt_application_Activator::m_Context = 0;

void org_mitk_gui_qt_application_Activator::start(ctkPluginContext* context)
{
  this->m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkGeneralPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkEditorsPreferencePage, context)

  QFile file(":/org.mitk.gui.qt.common/StateMachine.xml");
  if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text) )
  {
    QByteArray contents = file.readAll();
    QString string(contents);
    file.close();

    mitk::GlobalInteraction::GetInstance()->Initialize("global", string.toStdString());
  }
  else throw std::exception();

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


