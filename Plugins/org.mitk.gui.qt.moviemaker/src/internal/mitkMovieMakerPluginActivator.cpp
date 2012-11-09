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
#include "mitkMovieMakerPluginActivator.h"

#include "QmitkMovieMaker.h"
#include "QmitkScreenshotMaker.h"

#include <QtPlugin>

namespace mitk {

  void MovieMakerPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMovieMaker, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkScreenshotMaker, context)
  }

  void MovieMakerPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_moviemaker, mitk::MovieMakerPluginActivator)
