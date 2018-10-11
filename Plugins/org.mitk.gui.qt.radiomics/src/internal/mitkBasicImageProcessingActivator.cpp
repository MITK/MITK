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
