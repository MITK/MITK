/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkCommonActivator.h"

#include "../QmitkStdMultiWidgetEditor.h"
#include "QmitkStdMultiWidgetEditorPreferencePage.h"
#include "QmitkGeneralPreferencePage.h"
#include "QmitkEditorsPreferencePage.h"

#include <mitkGlobalInteraction.h>
#include <QmitkRegisterClasses.h>


void
QmitkCommonActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkStdMultiWidgetEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkStdMultiWidgetEditorPreferencePage, context)
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

void
QmitkCommonActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_common, QmitkCommonActivator)
