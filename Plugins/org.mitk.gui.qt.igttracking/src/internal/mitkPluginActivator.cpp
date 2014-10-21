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


#include <QtPlugin>

#include "QmitkMITKIGTNavigationToolManagerView.h"
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkNavigationDataPlayerView.h"

//#include <mitkPersistenceService.h> //Workaround for bug in persistence module (see bug 16643 for details)
                                    //CAN BE REMOVED WHEN THE BUG IS FIXED

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
 // mitk::PersistenceService::LoadModule(); //Workaround for bug in persistence module (see bug 16643 for details)
                                          //CAN BE REMOVED WHEN THE BUG IS FIXED


  BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKIGTNavigationToolManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS( QmitkMITKIGTTrackingToolboxView , context)
  BERRY_REGISTER_EXTENSION_CLASS( QmitkNavigationDataPlayerView , context)


}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_igttracking, mitk::PluginActivator)
#endif
