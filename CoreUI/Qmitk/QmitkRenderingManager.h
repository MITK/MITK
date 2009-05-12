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
#include <QObject>
#include <QEvent>

class QmitkRenderingManagerInternal;
class QmitkRenderingManagerFactory;

/**
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

  virtual bool event( QEvent *event );

protected:
  itkFactorylessNewMacro(Self);

  QmitkRenderingManager();

  virtual void GenerateRenderingRequestEvent();


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

  QmitkRenderingRequestEvent()
  : QEvent( (QEvent::Type) RenderingRequest ) {};
};

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
