/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "org_mitk_gui_qt_matchpoint_manipulator_Activator.h"

#include "QmitkMatchPointRegistrationManipulator.h"

ctkPluginContext* org_mitk_gui_qt_matchpoint_manipulator_Activator::m_Context = nullptr;

void org_mitk_gui_qt_matchpoint_manipulator_Activator::start(
    ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(QmitkMatchPointRegistrationManipulator, context)

	m_Context = context;
}

void org_mitk_gui_qt_matchpoint_manipulator_Activator::stop(
    ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = nullptr;
}

ctkPluginContext* org_mitk_gui_qt_matchpoint_manipulator_Activator::GetContext()
{
	return m_Context;
}
