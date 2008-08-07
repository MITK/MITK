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


#ifndef QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkRenderingManager.h"
#include <qobject.h>

class QTimer;
class QmitkRenderingManagerInternal;
class QmitkRenderingManagerFactory;

/**
 * \brief Qt specific implementation of mitk::RenderingManager.
 *
 * This implementation uses a QTimer object to realize the RenderWindow
 * update timing. The execution of pending updates is controlled by the
 * timer.
 *
 * Generally, rendering execution is controlled by the timer. To make sure
 * that the execution is done regularly even if the timer is blocked for
 * some reason (e.g., timer events have lower priority than other events
 * which are blocking the event queue), rendering is also executed when
 * the mouse is moved with pressed left button over the render windows.
 * See Qmitk::RenderWindow::mouseMoveEvent(...)
 *
 * \ingroup Renderer
 */
class QMITK_EXPORT QmitkRenderingManager : public QObject, public mitk::RenderingManager
{
  Q_OBJECT
public:
  mitkClassMacro( QmitkRenderingManager, mitk::RenderingManager );
  virtual ~QmitkRenderingManager();

  virtual void DoMonitorRendering();
  virtual void DoFinishAbortRendering();


protected:
  itkFactorylessNewMacro(Self);

  QmitkRenderingManager();

  virtual void RestartTimer();

  virtual void StopTimer();


private:

  QmitkRenderingManagerInternal* m_QmitkRenderingManagerInternal;

  friend class QmitkRenderingManagerFactory;
};

class QMITK_EXPORT QmitkRenderingManagerInternal : public QObject
{
  Q_OBJECT

public:
  friend class QmitkRenderingManager;

  virtual ~QmitkRenderingManagerInternal();

protected:
  QmitkRenderingManagerInternal();

protected slots:

  void RestartTimer();

  void StopTimer();

  void QUpdateCallback();

private:
  QmitkRenderingManager* m_QmitkRenderingManager;
  QTimer *m_Timer;
  static QmitkRenderingManager* m_Instance;

};

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
