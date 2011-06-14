/*=========================================================================

 Program:   BlueBerry Platform
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

#include "QmitkCommonExtPlugin.h"

#include <QmitkExtRegisterClasses.h>

#include "../QmitkExtPreferencePage.h"
#include "../QmitkInputDevicesPrefPage.h"

#include <QtPlugin>

ctkPluginContext* QmitkCommonExtPlugin::_context = 0;

void QmitkCommonExtPlugin::start(ctkPluginContext* context)
{
  this->_context = context;

  QmitkExtRegisterClasses();
  
  BERRY_REGISTER_EXTENSION_CLASS(QmitkExtPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkInputDevicesPrefPage, context)
}

void QmitkCommonExtPlugin::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  this->_context = 0;
}

ctkPluginContext* QmitkCommonExtPlugin::getContext()
{
  return _context;
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_ext, QmitkCommonExtPlugin)
