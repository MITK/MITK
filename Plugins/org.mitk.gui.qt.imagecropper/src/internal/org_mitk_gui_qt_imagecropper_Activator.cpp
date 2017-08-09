/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkBoundingShapeObjectFactory.h>
#include "org_mitk_gui_qt_imagecropper_Activator.h"

#include <QtPlugin>

#include "QmitkImageCropper.h"
#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

namespace mitk {

  void org_mitk_gui_qt_imagecropper_Activator::start(ctkPluginContext* context)
  {
    RegisterBoundingShapeObjectFactory();
    BERRY_REGISTER_EXTENSION_CLASS(QmitkImageCropper, context)
  }

  void org_mitk_gui_qt_imagecropper_Activator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(org_mitk_gui_qt_imagecropper, mitk::PluginActivator)
#endif