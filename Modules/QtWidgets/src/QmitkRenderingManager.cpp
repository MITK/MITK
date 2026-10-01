/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderingManager.h>

#include <mitkBaseRenderer.h>
#include <mitkGeometry3D.h>
#include <mitkSliceNavigationController.h>

#include <QApplication>
#include <QTimer>

QmitkRenderingManager::QmitkRenderingManager()
  : m_AnimationTimer(new QTimer(this))
{
  pendingTimerCallbacks = 0;

  m_AnimationTimer->setTimerType(Qt::PreciseTimer);
  connect(m_AnimationTimer, &QTimer::timeout, this, [this]() { this->ExecuteAnimationFrame(); });
}

void QmitkRenderingManager::DoMonitorRendering()
{
}

void QmitkRenderingManager::DoFinishAbortRendering()
{
}

QmitkRenderingManager::~QmitkRenderingManager()
{
}

void QmitkRenderingManager::GenerateRenderingRequestEvent()
{
  QApplication::postEvent(this, new QmitkRenderingRequestEvent);
}

void QmitkRenderingManager::StartOrResetTimer()
{
  QTimer::singleShot(200, this, SLOT(TimerCallback()));
  pendingTimerCallbacks++;
}

void QmitkRenderingManager::TimerCallback()
{
  if (!--pendingTimerCallbacks)
    this->ExecutePendingHighResRenderingRequest();
}

void QmitkRenderingManager::StartAnimationTimer(std::chrono::milliseconds interval)
{
  m_AnimationTimer->start(interval);
}

void QmitkRenderingManager::StopAnimationTimer()
{
  m_AnimationTimer->stop();
}

bool QmitkRenderingManager::event(QEvent *event)
{
  if (event->type() == (QEvent::Type)QmitkRenderingRequestEvent::RenderingRequest)
  {
    // Directly process all pending rendering requests
    this->ExecutePendingRequests();
    return true;
  }

  return false;
}
