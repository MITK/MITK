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

#include "org_mitk_gui_qt_stdmultiwidgeteditor_Activator.h"

#include "../QmitkStdMultiWidgetEditor.h"
#include "QmitkStdMultiWidgetEditorPreferencePage.h"


void
org_mitk_gui_qt_stdmultiwidgeteditor_Activator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkStdMultiWidgetEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkStdMultiWidgetEditorPreferencePage, context)
}

void
org_mitk_gui_qt_stdmultiwidgeteditor_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_stdmultiwidgeteditor, org_mitk_gui_qt_stdmultiwidgeteditor_Activator)
