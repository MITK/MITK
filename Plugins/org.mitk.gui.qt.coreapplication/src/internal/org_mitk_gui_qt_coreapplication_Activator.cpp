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


#include "org_mitk_gui_qt_coreapplication_Activator.h"

#include "QmitkCoreApplication.h"
#include "QmitkDefaultPerspective.h"

#include <QtPlugin>

namespace mitk {

void org_mitk_gui_qt_coreapplication_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCoreApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDefaultPerspective, context)
}

void org_mitk_gui_qt_coreapplication_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_coreapplication, mitk::org_mitk_gui_qt_coreapplication_Activator)


