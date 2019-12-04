/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "org_mitk_gui_qt_chartExample_Activator.h"
#include "QmitkChartExampleView.h"

namespace mitk
{
  void org_mitk_gui_qt_chartExample_Activator::start(ctkPluginContext *context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkChartExampleView, context)
  }

  void org_mitk_gui_qt_chartExample_Activator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
