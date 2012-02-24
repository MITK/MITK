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


#ifndef QmitkDicomDataEventPublisher_H
#define QmitkDicomDataEventPublisher_H

#include <ctkServiceReference.h>
#include <ctkPluginContext.h>
#include <Libs/PluginFramework/service/event/ctkEventAdmin.h>
#include <Libs/PluginFramework/service/event/ctkEvent.h>

class QmitkDicomDataEventPublisher
{
    public:

        QmitkDicomDataEventPublisher();

        virtual ~QmitkDicomDataEventPublisher();

        /// @brief sends an ctkEvent default asynchronously
        void SendEvent(const ctkEvent& ctkEvent, bool synchronously = false );

        /// @brief sets the event admin from given plugin context
        void SetEventAdmin(ctkPluginContext* context);
		
	private:

        /// @brief sets the service reference from given plugin context
        void SetServiceReference(ctkPluginContext* context);

        ctkServiceReference* m_ServiceReference;
        ctkEventAdmin* m_EventAdmin;

};
#endif // QmitkDicomDataEventPublisher_H