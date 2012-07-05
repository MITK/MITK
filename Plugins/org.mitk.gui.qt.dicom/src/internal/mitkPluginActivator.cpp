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

#include "QmitkDicomEditor.h"
//#include "QmitkDicomPreferencePage.h"

namespace mitk {

ctkPluginContext* PluginActivator::pluginContext = 0;

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomEditor, context)
  //BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomPreferencePage, context)
  pluginContext = context;
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  pluginContext = NULL;
}
ctkPluginContext* PluginActivator::getContext()
{
    return pluginContext;
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_dicom, mitk::PluginActivator)
