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


#include "org_mitk_example_gui_customviewer_views_Activator.h"

#include "DicomView.h"
#include "SimpleRenderWindowView.h"

#include <QtPlugin>

ctkPluginContext* org_mitk_example_gui_customviewer_views_Activator::PluginContext = 0;

void org_mitk_example_gui_customviewer_views_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(DicomView, context)
  BERRY_REGISTER_EXTENSION_CLASS(SimpleRenderWindowView, context)
  PluginContext = context;
}

void org_mitk_example_gui_customviewer_views_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  PluginContext = 0;
}

ctkPluginContext* org_mitk_example_gui_customviewer_views_Activator::GetPluginContext()
{
  return PluginContext;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_example_gui_customviewer_views, org_mitk_example_gui_customviewer_views_Activator)
#endif

