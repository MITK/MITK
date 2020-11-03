/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKDEFAULTDROPTARGETLISTENER_H
#define QMITKDEFAULTDROPTARGETLISTENER_H

#include <org_mitk_gui_qt_application_Export.h>

#include <berryIDropTargetListener.h>

#include <QScopedPointer>

class QmitkDefaultDropTargetListenerPrivate;

/**
 * \ingroup org_mitk_gui_qt_application
 */
class MITK_QT_APP QmitkDefaultDropTargetListener : public berry::IDropTargetListener
{
public:
  QmitkDefaultDropTargetListener();
  ~QmitkDefaultDropTargetListener() override;

  Events::Types GetDropTargetEventTypes() const override;

  void DropEvent(QDropEvent* event) override;

private:

  const QScopedPointer<QmitkDefaultDropTargetListenerPrivate> d;

};

#endif // QMITKDEFAULTDROPTARGETLISTENER_H
