/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_photoacousticsimulation_Activator.h"
#include "PASimulator.h"

void mitk::org_mitk_gui_qt_photoacousticsimulation_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(PASimulator, context)
}

void mitk::org_mitk_gui_qt_photoacousticsimulation_Activator::stop(ctkPluginContext*)
{
}
