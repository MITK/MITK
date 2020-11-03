/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ
#define QMITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ

#include "MitkQtWidgetsExtExports.h"
#include "mitkCallbackFromGUIThread.h"

#include <QObject>

/*!
  \brief Qt specific implementation of mitk::CallbackFromGUIThreadImplementation
*/
class MITKQTWIDGETSEXT_EXPORT QmitkCallbackFromGUIThread : public QObject,
                                                           public mitk::CallbackFromGUIThreadImplementation
{
  Q_OBJECT

public:
  /// Change the current application cursor
  void CallThisFromGUIThread(itk::Command *, itk::EventObject *) override;

  QmitkCallbackFromGUIThread();
  ~QmitkCallbackFromGUIThread() override;

  bool event(QEvent *e) override;

protected:
private:
};

#endif
