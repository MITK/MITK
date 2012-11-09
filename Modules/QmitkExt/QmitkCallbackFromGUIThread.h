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

#ifndef QMITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ
#define QMITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ

#include "mitkCallbackFromGUIThread.h"
#include "QmitkExtExports.h"

#include <QObject>

/*!
  \brief Qt specific implementation of mitk::CallbackFromGUIThreadImplementation
*/
class QmitkExt_EXPORT QmitkCallbackFromGUIThread : public QObject, public mitk::CallbackFromGUIThreadImplementation
{

  Q_OBJECT

  public:

    /// Change the current application cursor
    virtual void CallThisFromGUIThread(itk::Command*, itk::EventObject*);

    QmitkCallbackFromGUIThread();
    virtual ~QmitkCallbackFromGUIThread();

    virtual bool event( QEvent* e );

  protected:
  private:
};

#endif

