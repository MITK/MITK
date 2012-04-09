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


#include "org_mitk_gui_common_Activator.h"

#include <QtPlugin>

namespace mitk {

ctkPluginContext* org_mitk_gui_common_Activator::m_Context = 0;

void org_mitk_gui_common_Activator::start(ctkPluginContext* context)
{
  m_Context = context;
}

void org_mitk_gui_common_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  m_Context = 0;
}

ctkPluginContext *org_mitk_gui_common_Activator::GetContext()
{
  return m_Context;
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_common, mitk::org_mitk_gui_common_Activator)


