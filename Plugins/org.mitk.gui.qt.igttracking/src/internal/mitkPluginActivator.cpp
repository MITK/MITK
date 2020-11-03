/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkMITKIGTNavigationToolManagerView.h"
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkNavigationDataPlayerView.h"
#include "QmitkIGTNavigationToolCalibration.h"
#include "QmitkIGTFiducialRegistration.h"

//#include <mitkPersistenceService.h> //Workaround for bug in persistence module (see bug 16643 for details)
                                    //CAN BE REMOVED WHEN THE BUG IS FIXED

namespace mitk {

  ctkPluginContext* PluginActivator::m_Context = nullptr;

void PluginActivator::start(ctkPluginContext* context)
{
 // mitk::PersistenceService::LoadModule(); //Workaround for bug in persistence module (see bug 16643 for details)
                                          //CAN BE REMOVED WHEN THE BUG IS FIXED
  m_Context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKIGTNavigationToolManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKIGTTrackingToolboxView , context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkNavigationDataPlayerView , context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTNavigationToolCalibration , context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTFiducialRegistration, context)


}

void PluginActivator::stop(ctkPluginContext* context)
{
  m_Context = nullptr;

  Q_UNUSED(context)
}

ctkPluginContext *PluginActivator::GetContext()
{
  return m_Context;
}

}
