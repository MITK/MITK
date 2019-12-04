/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include <MitkQtWidgetsExports.h>

#include "mitkRenderingManager.h"
#include <QEvent>
#include <QObject>

class QmitkRenderingManagerInternal;
class QmitkRenderingManagerFactory;

/**
 * \ingroup QmitkModule
 * \brief Qt specific implementation of mitk::RenderingManager.
 *
 * This implementation defines a QmitkRenderingRequestEvent to realize the
 * rendering request process. The event must be handled by the Qmitk
 * interface to Qt (QmitkRenderWindow).
 *
 * Note: it may be necessary to remove all pending RenderingRequestEvents
 * from the system's event processing pipeline during system shutdown to
 * make sure that dangling events do not lead to unexpected behavior.
 *
 */
class MITKQTWIDGETS_EXPORT QmitkRenderingManager : public QObject, public mitk::RenderingManager
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkRenderingManager, mitk::RenderingManager);
  ~QmitkRenderingManager() override;

  void DoMonitorRendering() override;
  void DoFinishAbortRendering() override;

  bool event(QEvent *event) override;

protected:
  itkFactorylessNewMacro(Self);

  QmitkRenderingManager();

  void GenerateRenderingRequestEvent() override;

  void StartOrResetTimer() override;

  int pendingTimerCallbacks;

protected slots:

  void TimerCallback();

private:
  friend class QmitkRenderingManagerFactory;
};

class QmitkRenderingRequestEvent : public QEvent
{
public:
  enum Type
  {
    RenderingRequest = QEvent::MaxUser - 1024
  };

  QmitkRenderingRequestEvent() : QEvent((QEvent::Type)RenderingRequest){};
};

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
