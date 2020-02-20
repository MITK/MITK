/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_example_gui_selectionservicemitk_views_Activator.h"

#include "ListenerViewMitk.h"
#include "SelectionViewMitk.h"

void org_mitk_example_gui_selectionservicemitk_views_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(ListenerViewMitk, context)
  BERRY_REGISTER_EXTENSION_CLASS(SelectionViewMitk, context)
}

void org_mitk_example_gui_selectionservicemitk_views_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)
}
