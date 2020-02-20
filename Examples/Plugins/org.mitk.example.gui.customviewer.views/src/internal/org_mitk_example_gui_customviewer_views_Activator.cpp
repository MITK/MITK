/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_example_gui_customviewer_views_Activator.h"

#include "DicomView.h"
#include "SimpleRenderWindowView.h"

ctkPluginContext *org_mitk_example_gui_customviewer_views_Activator::PluginContext = nullptr;

void org_mitk_example_gui_customviewer_views_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(DicomView, context)
  BERRY_REGISTER_EXTENSION_CLASS(SimpleRenderWindowView, context)
  PluginContext = context;
}

void org_mitk_example_gui_customviewer_views_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)

  PluginContext = nullptr;
}

ctkPluginContext *org_mitk_example_gui_customviewer_views_Activator::GetPluginContext()
{
  return PluginContext;
}
