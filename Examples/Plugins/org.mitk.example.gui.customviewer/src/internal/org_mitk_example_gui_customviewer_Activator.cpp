/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_example_gui_customviewer_Activator.h"

#include "CustomViewer.h"
#include "DicomPerspective.h"
#include "ViewerPerspective.h"

ctkPluginContext *org_mitk_example_gui_customviewer_Activator::PluginContext = nullptr;

void org_mitk_example_gui_customviewer_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CustomViewer, context)
  BERRY_REGISTER_EXTENSION_CLASS(ViewerPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(DicomPerspective, context)
  PluginContext = context;
}

void org_mitk_example_gui_customviewer_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)

  PluginContext = nullptr;
}

ctkPluginContext *org_mitk_example_gui_customviewer_Activator::GetPluginContext()
{
  return PluginContext;
}
