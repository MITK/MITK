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


#ifndef DicomEventHandler_h
#define DicomEventHandler_h

#include <QObject>
#include <QString>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEvent.h>
#include <ctkPluginContext.h>

class DicomEventHandler : public QObject, public ctkEventHandler
{
    Q_OBJECT
    Q_INTERFACES(ctkEventHandler)
    public:

		DicomEventHandler();

		virtual ~DicomEventHandler();

        void handleEvent(const ctkEvent& ctkEvent);

        /*!
        \brief Registers the dicom event handler with its topic as a service
        \param eventTopic the specific topic which will be handeled
        \param filter you can filter events e.g. if you pass "(title=samplereport)" only events with a title samplereport will be processed. 
        */
        void Register(QString eventTopic, QString filter  = QString());

        void SetPluginContext(ctkPluginContext* context);

    private:

        ctkPluginContext* m_PluginContext;
};
#endif // QmitkDicomEventHandlerBuilder_h