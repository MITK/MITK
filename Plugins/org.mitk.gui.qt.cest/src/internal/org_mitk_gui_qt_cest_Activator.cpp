/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_cest_Activator.h"
#include "QmitkCESTStatisticsView.h"
#include "QmitkCESTNormalizeView.h"

void mitk::org_mitk_gui_qt_cest_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCESTStatisticsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCESTNormalizeView, context)
}

void mitk::org_mitk_gui_qt_cest_Activator::stop(ctkPluginContext*)
{
}
