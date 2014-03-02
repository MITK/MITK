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


#ifndef QmlMitkRenderingManager_h
#define QmlMitkRenderingManager_h

#include <QmlMitkExports.h>

#include "mitkRenderingManager.h"

#include <QObject>
#include <QEvent>

class QmlMitkRenderingManagerFactory;

/**
 * \ingroup QmlMitkModule
 * \brief Qt/Qml specific implementation of mitk::RenderingManager.
 *
 * This implementation defines a QmlMitkRenderingRequestEvent to realize the
 * rendering request process. The event is put into Qt's event loop to
 * receive it back in the GUI thread where we are allowed to do rendering.
 *
 */
class QmlMitk_EXPORT QmlMitkRenderingManager : public QObject, public mitk::RenderingManager
{
  Q_OBJECT

public:

  mitkClassMacro( QmlMitkRenderingManager, mitk::RenderingManager );
  virtual ~QmlMitkRenderingManager();

  virtual bool event( QEvent *event );

protected:

  itkFactorylessNewMacro(Self);

  QmlMitkRenderingManager();

  virtual void GenerateRenderingRequestEvent();

  virtual void StartOrResetTimer();

  int pendingTimerCallbacks;

 protected slots:

  void TimerCallback();

private:

  friend class QmlMitkRenderingManagerFactory;

  void MyUpdateExecutePendingRequests();
};

class QmlMitkRenderingRequestEvent : public QEvent
{
public:
  enum Type
  {
    RenderingRequest = QEvent::MaxUser - 1024
  };

  QmlMitkRenderingRequestEvent()
  : QEvent( (QEvent::Type) RenderingRequest ) {};
};

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
