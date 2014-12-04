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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_common, mitk::org_mitk_gui_common_Activator)
#endif
