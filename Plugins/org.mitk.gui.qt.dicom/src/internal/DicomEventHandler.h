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
#include <service/event/ctkEvent.h>

class DicomEventHandler : public QObject
{
    Q_OBJECT
public:

    DicomEventHandler();

    virtual ~DicomEventHandler();

    void SubscribeSlots();

    public slots:

        void OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent);

        void OnSignalRemoveSeriesFromStorage(const ctkEvent& ctkEvent);
};
#endif // QmitkDicomEventHandlerBuilder_h