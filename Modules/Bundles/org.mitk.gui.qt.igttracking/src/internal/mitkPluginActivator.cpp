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


#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "QmitkMITKIGTNavigationToolManagerView.h"
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkNavigationDataPlayerView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKIGTNavigationToolManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS( QmitkMITKIGTTrackingToolboxView , context)
  BERRY_REGISTER_EXTENSION_CLASS( QmitkNavigationDataPlayerView , context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_igttracking, mitk::PluginActivator)