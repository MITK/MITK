/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_FitGeneratorDemo_Activator.h"

#include "FitGeneratorDemoView.h"

    void org_mitk_gui_qt_fit_demo_Activator::start(ctkPluginContext* context)
	{
        BERRY_REGISTER_EXTENSION_CLASS(FitGeneratorDemoView, context);
	}

    void org_mitk_gui_qt_fit_demo_Activator::stop(ctkPluginContext* context)
	{
        Q_UNUSED(context);
	}
