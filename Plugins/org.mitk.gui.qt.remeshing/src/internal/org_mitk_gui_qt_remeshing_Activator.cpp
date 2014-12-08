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

#include "org_mitk_gui_qt_remeshing_Activator.h"
#include "QmitkRemeshingView.h"
#include <QtPlugin>

void mitk::org_mitk_gui_qt_remeshing_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRemeshingView, context);
}

void mitk::org_mitk_gui_qt_remeshing_Activator::stop(ctkPluginContext*)
{
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_remeshing, mitk::org_mitk_gui_qt_remeshing_Activator)
#endif
