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


#ifndef QMITKDEFAULTDROPTARGETLISTENER_H
#define QMITKDEFAULTDROPTARGETLISTENER_H

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIDropTargetListener.h>

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkDefaultDropTargetListener : public berry::IDropTargetListener
{
public:

  Events::Types GetDropTargetEventTypes() const;

  void DropEvent(QDropEvent* event);

};

#endif // QMITKDEFAULTDROPTARGETLISTENER_H
