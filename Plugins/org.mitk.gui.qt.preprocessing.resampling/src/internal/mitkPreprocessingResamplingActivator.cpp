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

#include "mitkPreprocessingResamplingActivator.h"
#include "QmitkPreprocessingResamplingView.h"

namespace mitk {

void PreprocessingResamplingActivator::start(ctkPluginContext* context)
{
   BERRY_REGISTER_EXTENSION_CLASS(QmitkPreprocessingResampling, context)
}

void PreprocessingResamplingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
