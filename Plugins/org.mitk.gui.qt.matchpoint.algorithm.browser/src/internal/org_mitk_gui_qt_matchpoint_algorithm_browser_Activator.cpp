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

#include "org_mitk_gui_qt_matchpoint_algorithm_browser_Activator.h"

#include "QmitkMatchPointBrowser.h"

ctkPluginContext* org_mitk_gui_qt_matchpoint_algorithm_browser_Activator::m_Context = 0;

void org_mitk_gui_qt_matchpoint_algorithm_browser_Activator::start(ctkPluginContext* context)
{
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMatchPointBrowser, context)

        m_Context = context;
}

void org_mitk_gui_qt_matchpoint_algorithm_browser_Activator::stop(ctkPluginContext* context)
{
    Q_UNUSED(context)

        m_Context = 0;
}

ctkPluginContext* org_mitk_gui_qt_matchpoint_algorithm_browser_Activator::GetContext()
{
    return m_Context;
}
