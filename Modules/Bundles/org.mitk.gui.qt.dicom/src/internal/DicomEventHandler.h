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
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEventHandler.h>

class DicomEventHandler : public QObject, public ctkEventHandler
{
 Q_OBJECT
 Q_INTERFACES(ctkEventHandler)
	public:

		DicomEventHandler();

		virtual ~DicomEventHandler();

};
#endif // QmitkDicomEventHandlerBuilder_h