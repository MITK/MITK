/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkProgressNotificationOverlay.h>
#include <QmitkProgressNotification.h>

#include <mitkCoreServices.h>
#include <mitkIProgressService.h>

#include <QEvent>
#include <QLabel>
#include <QMainWindow>
#include <QResizeEvent>
#include <QStatusBar>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
  constexpr int MAX_VISIBLE_NOTIFICATIONS = 5;
  constexpr int NOTIFICATION_WIDTH = 320;
  constexpr int MARGIN = 12;

  /**
   * Long operations that run on the GUI thread never reach the event loop, so
   * the overlay has to paint itself synchronously to be visible at all. Some
   * of them report hundreds of steps, though, where a synchronous repaint per
   * step would cost more than the operation itself.
   */
  constexpr qint64 REPAINT_INTERVAL_IN_MS = 40;
}

QmitkProgressNotificationOverlay::QmitkProgressNotificationOverlay(QWidget* parent)
  : QWidget(parent),
    m_Service(nullptr),
    m_Layout(new QVBoxLayout(this)),
    m_OverflowLabel(new QLabel(this))
{
  this->setObjectName(QStringLiteral("QmitkProgressNotificationOverlay"));

  // The application style sheet gives every widget an opaque background. Only
  // the cards should be visible, so the container explicitly asks for none.
  this->setStyleSheet(QStringLiteral("#QmitkProgressNotificationOverlay { background-color: transparent; }"));

  m_Layout->setContentsMargins(0, 0, 0, 0);
  m_Layout->setSpacing(4);

  m_OverflowLabel->setAlignment(Qt::AlignRight);
  m_OverflowLabel->hide();
  m_Layout->addWidget(m_OverflowLabel);

  this->hide();
  this->InstallEventFilterOnParent();

  m_Service = mitk::CoreServices::GetProgressService();

  if (nullptr != m_Service)
  {
    // Registering replays whatever is already running, so an overlay created
    // while an operation is in flight does not miss it.
    m_Service->AddListener(this);
  }
}

QmitkProgressNotificationOverlay::~QmitkProgressNotificationOverlay()
{
  if (nullptr != m_Service)
  {
    m_Service->RemoveListener(this);
    mitk::CoreServices::Unget(m_Service);
  }

  this->RemoveEventFilterFromParent();
}

void QmitkProgressNotificationOverlay::OnTaskUpdated(const mitk::ProgressTaskInfo& info)
{
  // Called from whichever thread reported the progress. AutoConnection keeps
  // the GUI-thread case synchronous, which is what lets a blocking operation
  // paint its own progress.
  QMetaObject::invokeMethod(this, [this, info]() {
    this->Apply(info);
  }, Qt::AutoConnection);
}

void QmitkProgressNotificationOverlay::Apply(const mitk::ProgressTaskInfo& info)
{
  if (m_Dismissed.contains(info.Id))
  {
    if (info.Finished)
    {
      m_Dismissed.remove(info.Id);
      m_Sequences.remove(info.Id);
    }

    return;
  }

  if (info.Sequence <= m_Sequences.value(info.Id, 0))
    return;

  m_Sequences.insert(info.Id, info.Sequence);

  auto* notification = m_Notifications.value(info.Id, nullptr);
  auto appeared = false;

  if (info.Finished)
  {
    m_Notifications.remove(info.Id);
    m_Sequences.remove(info.Id);

    if (nullptr == notification)
      return;

    notification->Finish(info);
  }
  else if (nullptr == notification)
  {
    notification = new QmitkProgressNotification(info, this);

    connect(notification, &QmitkProgressNotification::CancelRequested,
      this, &QmitkProgressNotificationOverlay::OnCancelRequested);
    connect(notification, &QmitkProgressNotification::Closed,
      this, &QmitkProgressNotificationOverlay::OnNotificationClosed);

    m_Notifications.insert(info.Id, notification);
    m_Layout->insertWidget(0, notification);

    appeared = true;
  }
  else
  {
    notification->Update(info);
  }

  this->RefreshVisibility();
  this->UpdatePosition();

  // A card that appears or disappears must not wait for the throttle, or a
  // short operation would never be seen at all.
  this->Repaint(appeared || info.Finished);
}

void QmitkProgressNotificationOverlay::OnCancelRequested(mitk::ProgressTaskId id)
{
  if (nullptr != m_Service)
    m_Service->RequestCancel(id);
}

void QmitkProgressNotificationOverlay::OnNotificationClosed(mitk::ProgressTaskId id)
{
  auto* notification = qobject_cast<QmitkProgressNotification*>(this->sender());

  if (nullptr == notification)
    return;

  if (m_Notifications.value(id, nullptr) == notification)
  {
    // The task is still running. Remember that its card was dismissed so the
    // next snapshot does not bring it back.
    m_Notifications.remove(id);
    m_Dismissed.insert(id);
  }

  m_Layout->removeWidget(notification);
  notification->deleteLater();

  this->RefreshVisibility();
  this->UpdatePosition();
}

void QmitkProgressNotificationOverlay::RefreshVisibility()
{
  int visibleCount = 0;
  int hiddenCount = 0;

  for (int i = 0; i < m_Layout->count(); ++i)
  {
    auto* notification = qobject_cast<QmitkProgressNotification*>(m_Layout->itemAt(i)->widget());

    if (nullptr == notification)
      continue;

    if (visibleCount < MAX_VISIBLE_NOTIFICATIONS)
    {
      notification->setVisible(true);
      ++visibleCount;
    }
    else
    {
      notification->setVisible(false);
      ++hiddenCount;
    }
  }

  m_OverflowLabel->setVisible(0 != hiddenCount);

  if (0 != hiddenCount)
    m_OverflowLabel->setText(QStringLiteral("%1 more...").arg(hiddenCount));

  this->setVisible(0 != visibleCount);
}

void QmitkProgressNotificationOverlay::UpdatePosition()
{
  auto* parent = this->parentWidget();

  if (nullptr == parent)
    return;

  m_Layout->activate();

  const auto width = std::min(NOTIFICATION_WIDTH, parent->width() - 2 * MARGIN);
  const auto height = std::min(this->sizeHint().height(), parent->height() - 2 * MARGIN);

  auto anchor = parent->height();

  if (auto* mainWindow = qobject_cast<QMainWindow*>(parent); nullptr != mainWindow)
  {
    auto* statusBar = mainWindow->statusBar();

    if (nullptr != statusBar && statusBar->isVisibleTo(mainWindow))
      anchor = statusBar->geometry().top();
  }

  this->resize(std::max(width, 0), std::max(height, 0));
  this->move(std::max(MARGIN, parent->width() - this->width() - MARGIN),
             std::max(MARGIN, anchor - this->height() - MARGIN));
}

void QmitkProgressNotificationOverlay::Repaint(bool force)
{
  if (!this->isVisible())
    return;

  if (!force && m_LastRepaint.isValid() && m_LastRepaint.elapsed() < REPAINT_INTERVAL_IN_MS)
    return;

  m_LastRepaint.restart();
  this->repaint();
}

bool QmitkProgressNotificationOverlay::event(QEvent* event)
{
  if (event->type() == QEvent::ParentAboutToChange)
  {
    this->RemoveEventFilterFromParent();
  }
  else if (event->type() == QEvent::ParentChange)
  {
    this->InstallEventFilterOnParent();
  }

  return QWidget::event(event);
}

bool QmitkProgressNotificationOverlay::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == this->parent())
  {
    switch (event->type())
    {
      // Show matters as much as Resize: leaving full screen recreates the
      // native window and applies the restored geometry one event loop turn
      // later, so a Resize alone leaves the overlay behind.
      case QEvent::Resize:
      case QEvent::Move:
      case QEvent::Show:
        this->UpdatePosition();
        break;

      case QEvent::ChildAdded:
        this->raise();
        break;

      default:
        break;
    }
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkProgressNotificationOverlay::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);

  this->raise();
  this->UpdatePosition();
}

void QmitkProgressNotificationOverlay::InstallEventFilterOnParent()
{
  if (nullptr == this->parent())
    return;

  this->parent()->installEventFilter(this);
  this->raise();
}

void QmitkProgressNotificationOverlay::RemoveEventFilterFromParent()
{
  if (nullptr == this->parent())
    return;

  this->parent()->removeEventFilter(this);
}
