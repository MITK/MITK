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


#include "org_mitk_example_gui_testapplication_Activator.h"

#include "TestApplication.h"
#include "TestPerspective.h"
#include "TestPerspective2.h"
#include "MinimalView.h"
#include "TestView.h"
//#include "QmitkDataManagerView.h"
#include "SimpleRenderWindowView.h"

#include <QtPlugin>

ctkPluginContext* org_mitk_example_gui_testapplication_Activator::PluginContext = 0;

void org_mitk_example_gui_testapplication_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(TestApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(TestPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(TestPerspective2, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalView, context)
  BERRY_REGISTER_EXTENSION_CLASS(TestView, context)
  //BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(SimpleRenderWindowView, context)
  PluginContext = context;
}

void org_mitk_example_gui_testapplication_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  PluginContext = 0;
}

ctkPluginContext* org_mitk_example_gui_testapplication_Activator::GetPluginContext()
{
  return PluginContext;
}

Q_EXPORT_PLUGIN2(org_mitk_example_gui_testapplication, org_mitk_example_gui_testapplication_Activator)


