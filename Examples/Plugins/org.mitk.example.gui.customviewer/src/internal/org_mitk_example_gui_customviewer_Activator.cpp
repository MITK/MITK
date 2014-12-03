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


#include "org_mitk_example_gui_customviewer_Activator.h"

#include "CustomViewer.h"
#include "ViewerPerspective.h"
#include "DicomPerspective.h"

#include <QtPlugin>

ctkPluginContext* org_mitk_example_gui_customviewer_Activator::PluginContext = 0;

void org_mitk_example_gui_customviewer_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CustomViewer, context)
  BERRY_REGISTER_EXTENSION_CLASS(ViewerPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(DicomPerspective, context)
  PluginContext = context;
}

void org_mitk_example_gui_customviewer_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  PluginContext = 0;
}

ctkPluginContext* org_mitk_example_gui_customviewer_Activator::GetPluginContext()
{
  return PluginContext;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_example_gui_customviewer, org_mitk_example_gui_customviewer_Activator)
#endif

