/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_ConcentrationCurveConverterView_Activator.h"

#include "ConcentrationCurveConverterView.h"

  void org_mitk_ConcentrationCurveConverterView_Activator::start(ctkPluginContext* context)
	{
        BERRY_REGISTER_EXTENSION_CLASS(ConcentrationCurveConverterView, context);
	}

    void org_mitk_ConcentrationCurveConverterView_Activator::stop(ctkPluginContext* context)
	{
        Q_UNUSED(context);
	}
