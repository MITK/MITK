/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBasicImageProcessingActivator.h"
#include "QmitkRadiomicsStatisticView.h"
#include "QmitkRadiomicsTransformationView.h"
#include "QmitkRadiomicsArithmetricView.h"
#include "QmitkRadiomicsMaskProcessingView.h"

namespace mitk {

void RadiomicsStatisticActivator::start(ctkPluginContext* context)
{
   BERRY_REGISTER_EXTENSION_CLASS(QmitkRadiomicsStatistic, context)
   BERRY_REGISTER_EXTENSION_CLASS(QmitkRadiomicsTransformation, context)
   BERRY_REGISTER_EXTENSION_CLASS(QmitkRadiomicsArithmetric, context)
   BERRY_REGISTER_EXTENSION_CLASS(QmitkRadiomicsMaskProcessing, context)
}

void RadiomicsStatisticActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
