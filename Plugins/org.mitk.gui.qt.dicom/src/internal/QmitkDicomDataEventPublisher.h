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

#include <ctkPluginContext.h>
#include <QObject>

class QmitkDicomDataEventPublisher : public QObject
{
    Q_OBJECT
    public:

        QmitkDicomDataEventPublisher();

        virtual ~QmitkDicomDataEventPublisher();

        /// @brief sets the event admin from given plugin context
        void PublishSignals(ctkPluginContext* context);

        void AddSeriesToDataManagerEvent(const ctkDictionary& properties);

        void RemoveSeriesFromStorageEvent(const ctkDictionary& properties);

    signals:
        void SignalAddSeriesToDataManager(const ctkDictionary&);

        void SignalRemoveSeriesFromStorage(const ctkDictionary&);
};
#endif // QmitkDicomDataEventPublisher_H