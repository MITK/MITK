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


#include "org_mitk_example_gui_extensionpointdefinition_Activator.h"

#include "ExtensionPointDefinition.h"
#include "MinimalPerspective.h"
#include "MinimalView.h"

#include <QtPlugin>

void org_mitk_example_gui_extensionpointdefinition_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(ExtensionPointDefinition, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalView, context)
}

void org_mitk_example_gui_extensionpointdefinition_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

ChangeTextRegistry* org_mitk_example_gui_extensionpointdefinition_Activator::getChangeTextRegistry()
{
  static ChangeTextRegistry registry;
  return &registry;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_example_gui_extensionpointdefinition, org_mitk_example_gui_extensionpointdefinition_Activator)
#endif
