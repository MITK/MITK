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
#include "MinimalView.h"
#include "TestView.h"

#include <QtPlugin>

void org_mitk_example_gui_testapplication_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(TestApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(TestPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalView, context)
  BERRY_REGISTER_EXTENSION_CLASS(TestView, context)
}

void org_mitk_example_gui_testapplication_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

Q_EXPORT_PLUGIN2(org_mitk_example_gui_testapplication, org_mitk_example_gui_testapplication_Activator)


