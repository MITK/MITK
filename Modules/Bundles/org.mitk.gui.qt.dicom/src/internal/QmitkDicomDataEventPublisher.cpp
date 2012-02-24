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

#include "QmitkDicomDataEventPublisher.h"

QmitkDicomDataEventPublisher::QmitkDicomDataEventPublisher()
{
}

QmitkDicomDataEventPublisher::~QmitkDicomDataEventPublisher()
{
    delete m_EventAdmin;
}

void QmitkDicomDataEventPublisher::SendEvent(const ctkEvent& ctkEvent,bool synchronously)
{
    if(synchronously)
    {
        m_EventAdmin->sendEvent(ctkEvent);
    }else{
        m_EventAdmin->postEvent(ctkEvent);
    }
}

void QmitkDicomDataEventPublisher::SetEventAdmin(ctkPluginContext* context)
{
    SetServiceReference(context);
    if(m_ServiceReference)
    {
        m_EventAdmin = context->getService<ctkEventAdmin>(m_ServiceReference);
    }
}

void QmitkDicomDataEventPublisher::SetServiceReference(ctkPluginContext* context)
{
    m_ServiceReference = context->getServiceReference<ctkEventAdmin>();
}