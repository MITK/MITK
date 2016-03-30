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

#include "org_mitk_matchpoint_core_helper_Activator.h"

#include "MatchPointBrowserPreferencesPage.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"


ctkPluginContext* org_mitk_matchpoint_core_helper_Activator::m_Context = nullptr;
org_mitk_matchpoint_core_helper_Activator*
org_mitk_matchpoint_core_helper_Activator::m_Instance = nullptr;

org_mitk_matchpoint_core_helper_Activator::org_mitk_matchpoint_core_helper_Activator()
{
    m_Instance = this;
}

org_mitk_matchpoint_core_helper_Activator::~org_mitk_matchpoint_core_helper_Activator()
{
    m_Instance = nullptr;
}

void org_mitk_matchpoint_core_helper_Activator::start(ctkPluginContext* context)
{
    BERRY_REGISTER_EXTENSION_CLASS(MatchPointBrowserPreferencesPage, context)

        this->m_Context = context;

    QmitkNodeDescriptorManager* manager = QmitkNodeDescriptorManager::GetInstance();

    mitk::NodePredicateDataType::Pointer isMITKRegistrationWrapper =
        mitk::NodePredicateDataType::New("MAPRegistrationWrapper");
    manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("MAPRegistrationWrapper"),
        QString(":/QmitkMatchPointCore/MAPRegData.png"), isMITKRegistrationWrapper, manager));
}

void org_mitk_matchpoint_core_helper_Activator::stop(ctkPluginContext* context)
{
    Q_UNUSED(context)

        this->m_Context = NULL;
}

ctkPluginContext* org_mitk_matchpoint_core_helper_Activator::getContext()
{
    return m_Context;
}

org_mitk_matchpoint_core_helper_Activator*
org_mitk_matchpoint_core_helper_Activator::getDefault()
{
    return m_Instance;
}
