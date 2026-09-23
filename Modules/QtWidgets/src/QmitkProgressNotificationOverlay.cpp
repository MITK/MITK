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
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <chrono>

namespace
{
  constexpr int MAX_VISIBLE_NOTIFICATIONS = 5;

  /**
   * How many tasks the overlay keeps a sequence number for. Bounds what is
   * remembered about tasks that have already ended, which is what rejects a
   * snapshot delivered after the one that finished them.
   */
  constexpr int MAX_REMEMBERED_SEQUENCES = 256;

  constexpr int NOTIFICATION_WIDTH = 320;
  constexpr int MARGIN = 12;

  /**
   * Long operations that run on the GUI thread never reach the event loop, so
   * the overlay has to paint itself synchronously to be visible at all. Some
   * of them report hundreds of steps, though, where a synchronous repaint per
   * step would cost more than the operation itself.
   */
  constexpr qint64 REPAINT_INTERVAL_IN_MS = 40;

  /**
   * How long an operation has to run before it is worth telling the user
   * about. Most operations are over in a few milliseconds, and announcing
   * every one of them buries the ones that actually take time.
   */
  constexpr qint64 SHOW_DELAY_IN_MS = 1000;

  /** \brief How long the task has been running, in milliseconds. */
  qint64 TaskAgeInMs(const mitk::ProgressTaskInfo& info)
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - info.StartTime).count();
  }
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
  // Before anything else, including the dismissal bookkeeping below: a
  // snapshot that lost its race is not allowed to undo what a newer one
  // already established.
  if (info.Sequence <= m_Sequences.value(info.Id, 0))
    return;

  // Kept once the task has finished, rather than erased with the rest of its
  // state. It is the only thing that tells a snapshot which overtook the final
  // one that it is stale, and without it such a snapshot raises a card for a
  // task that can never send another and so can never take it down again.
  m_Sequences.insert(info.Id, info.Sequence);

  // Ids only ever increase, so the lowest ones belong to the tasks that ended
  // longest ago. Once this many newer tasks have been seen, nothing can still
  // be in flight for them and remembering them buys nothing.
  //
  // Never dropped for a task the overlay is still tracking, though: those are
  // exactly the ones a late snapshot can still arrive for, and forgetting one
  // lets that snapshot raise a card the task can no longer take down again.
  // Beyond that the bound wins over the guarantee, so such a card is left for
  // the user to dismiss, which its close button always allows.
  while (m_Sequences.size() > MAX_REMEMBERED_SEQUENCES)
  {
    const auto oldest = m_Sequences.begin();

    if (m_Notifications.contains(oldest.key())
      || m_Pending.contains(oldest.key())
      || m_Dismissed.contains(oldest.key()))
    {
      break;
    }

    m_Sequences.erase(oldest);
  }

  if (m_Dismissed.contains(info.Id))
  {
    if (info.Finished)
      m_Dismissed.remove(info.Id);

    return;
  }

  auto* notification = m_Notifications.value(info.Id, nullptr);

  if (nullptr == notification)
  {
    if (info.Finished)
    {
      // Over before it was worth mentioning.
      m_Pending.remove(info.Id);
      return;
    }

    auto pending = m_Pending.find(info.Id);

    if (pending == m_Pending.end())
    {
      m_Pending.insert(info.Id, info);

      const auto remaining = SHOW_DELAY_IN_MS - TaskAgeInMs(info);

      if (remaining <= 0)
      {
        // A snapshot can reach us long after it was taken, because whatever
        // kept this thread from its event loop also kept it from the delivery.
        // The task has then already outlived the delay and is shown at once.
        this->ShowPendingTask(info.Id);
        return;
      }

      // Covers a task that goes quiet after starting. A task that keeps
      // reporting is picked up by the age check below instead, which also
      // works while a blocking operation starves the event loop.
      QTimer::singleShot(remaining, this, [this, id = info.Id]()
        {
          this->ShowPendingTask(id);
        });

      return;
    }

    *pending = info;

    if (TaskAgeInMs(info) >= SHOW_DELAY_IN_MS)
      this->ShowPendingTask(info.Id);

    return;
  }

  if (info.Finished)
  {
    m_Notifications.remove(info.Id);

    notification->Finish(info);
  }
  else
  {
    notification->Update(info);
  }

  this->RefreshVisibility();
  this->UpdatePosition();

  // A card that disappears must not wait for the throttle, or the completed
  // state would never be drawn.
  this->Repaint(info.Finished);
}

void QmitkProgressNotificationOverlay::ShowPendingTask(mitk::ProgressTaskId id)
{
  auto pending = m_Pending.find(id);

  if (pending == m_Pending.end())
    return;

  const auto info = *pending;
  m_Pending.erase(pending);

  this->PruneFinishedNotifications();

  auto* notification = new QmitkProgressNotification(info, this);

  connect(notification, &QmitkProgressNotification::CancelRequested,
    this, &QmitkProgressNotificationOverlay::OnCancelRequested);
  connect(notification, &QmitkProgressNotification::Closed,
    this, &QmitkProgressNotificationOverlay::OnNotificationClosed);

  m_Notifications.insert(id, notification);
  m_Layout->insertWidget(0, notification);

  this->RefreshVisibility();
  this->UpdatePosition();
  this->Repaint(true);
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

  this->RemoveNotification(notification);

  this->RefreshVisibility();
  this->UpdatePosition();
}

void QmitkProgressNotificationOverlay::RemoveNotification(QmitkProgressNotification* notification)
{
  // removeWidget() only takes it out of the layout. It stays a visible child
  // at its last geometry until deleteLater() runs, which is a turn of the event
  // loop away, and longer while an operation is keeping the loop busy.
  notification->hide();

  m_Layout->removeWidget(notification);
  notification->deleteLater();
}

void QmitkProgressNotificationOverlay::PruneFinishedNotifications()
{
  // A finished card leaves the stack on a timer and then a fade, and neither of
  // those runs while an operation on this thread keeps the event loop busy. A
  // run of short operations therefore piles up completed cards, and they take
  // the room the one still running needs: it ends up created hidden and counted
  // as "1 more..." behind bars that are all full. So they are taken out as soon
  // as something else needs the space, oldest first.
  for (int i = m_Layout->count() - 1; 0 <= i; --i)
  {
    if (this->CountNotifications() < MAX_VISIBLE_NOTIFICATIONS)
      return;

    auto* notification = qobject_cast<QmitkProgressNotification*>(m_Layout->itemAt(i)->widget());

    if (nullptr != notification && notification->IsFinished())
      this->RemoveNotification(notification);
  }
}

int QmitkProgressNotificationOverlay::CountNotifications() const
{
  int count = 0;

  for (int i = 0; i < m_Layout->count(); ++i)
  {
    if (nullptr != qobject_cast<QmitkProgressNotification*>(m_Layout->itemAt(i)->widget()))
      ++count;
  }

  return count;
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
    // Not statusBar(), which is a factory: it installs an empty status bar on
    // a window that has none, and so can never return nullptr.
    if (m_StatusBar.isNull())
      m_StatusBar = mainWindow->findChild<QStatusBar*>(QString(), Qt::FindDirectChildrenOnly);

    if (!m_StatusBar.isNull() && m_StatusBar->isVisibleTo(mainWindow))
      anchor = m_StatusBar->geometry().top();
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
