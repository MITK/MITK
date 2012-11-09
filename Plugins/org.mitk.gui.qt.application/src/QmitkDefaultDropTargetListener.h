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
