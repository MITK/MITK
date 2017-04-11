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
#include "mitkPluginActivator.h"

#include "src/internal/Perspectives/QmitkDiffusionImagingAppPerspective.h"
#include "src/internal/Perspectives/QmitkDiffusionDefaultPerspective.h"

#include "src/internal/QmitkDiffusionDicomImportView.h"
#include "src/internal/QmitkControlVisualizationPropertiesView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionDefaultPerspective, context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionDicomImport, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkControlVisualizationPropertiesView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
