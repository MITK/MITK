/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_example_gui_multipleperspectives_Activator.h"

#include "EmptyView1.h"
#include "EmptyView2.h"
#include "ExtendedPerspective.h"
#include "MinimumPerspective.h"
#include "MultiplePerspectives.h"

void org_mitk_example_gui_multipleperspectives_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(MultiplePerspectives, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimumPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(ExtendedPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(EmptyView1, context)
  BERRY_REGISTER_EXTENSION_CLASS(EmptyView2, context)
}

void org_mitk_example_gui_multipleperspectives_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)
}
