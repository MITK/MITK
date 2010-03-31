/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkDelKeyFilter.h"

#include <QKeyEvent>
#include <src/internal/QmitkDataManagerView.h>

QmitkDelKeyFilter::QmitkDelKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{

}

bool QmitkDelKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
  if (event->type() == QEvent::KeyPress) 
  {
    QmitkDataManagerView* _DataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if(keyEvent->key() == Qt::Key_Delete && _DataManagerView)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ActionRemoveTriggered(false);
      // return true: this means the delete key event is not send to the table
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}