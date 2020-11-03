/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_example_gui_selectionserviceqt_Activator.h"

#include "ExtendedPerspective.h"
#include "ListenerView.h"
#include "SelectionServiceQt.h"
#include "SelectionView.h"

void org_mitk_example_gui_selectionserviceqt_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(SelectionServiceQt, context)
  BERRY_REGISTER_EXTENSION_CLASS(ExtendedPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(ListenerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(SelectionView, context)
}

void org_mitk_example_gui_selectionserviceqt_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)
}
