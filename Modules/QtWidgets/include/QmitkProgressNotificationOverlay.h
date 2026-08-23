/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkProgressNotificationOverlay_h
#define QmitkProgressNotificationOverlay_h

#include <MitkQtWidgetsExports.h>

#include <mitkIProgressListener.h>

#include <QElapsedTimer>
#include <QHash>
#include <QMap>
#include <QSet>
#include <QWidget>

class QLabel;
class QVBoxLayout;
class QmitkProgressNotification;

namespace mitk
{
  class IProgressService;
}

/**
 * \ingroup QmitkModule
 * \brief Stack of progress notifications, floating above the status bar.
 *
 * Shows one card per running mitk::ProgressTask in the bottom-right corner of
 * its parent window, newest on top. Create one per application window and give
 * it the QMainWindow as parent; it registers itself with the progress service
 * and positions itself.
 *
 * The overlay is a plain child widget that stays out of the main window
 * layout, so it floats above whatever occupies that corner. Floating dock
 * widgets are separate top-level windows and will cover it.
 *
 * \sa QmitkProgressNotification
 * \sa mitk::IProgressService
 */
class MITKQTWIDGETS_EXPORT QmitkProgressNotificationOverlay : public QWidget, public mitk::IProgressListener
{
  Q_OBJECT

public:
  explicit QmitkProgressNotificationOverlay(QWidget* parent = nullptr);
  ~QmitkProgressNotificationOverlay() override;

  void OnTaskUpdated(const mitk::ProgressTaskInfo& info) override;

protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void Apply(const mitk::ProgressTaskInfo& info);
  void ShowPendingTask(mitk::ProgressTaskId id);
  void OnCancelRequested(mitk::ProgressTaskId id);
  void OnNotificationClosed(mitk::ProgressTaskId id);
  void RefreshVisibility();
  void UpdatePosition();
  void Repaint(bool force);
  void InstallEventFilterOnParent();
  void RemoveEventFilterFromParent();

  mitk::IProgressService* m_Service;
  QVBoxLayout* m_Layout;
  QLabel* m_OverflowLabel;

  QHash<mitk::ProgressTaskId, QmitkProgressNotification*> m_Notifications;

  /** \brief Tasks waiting out the delay before they get a card. */
  QHash<mitk::ProgressTaskId, mitk::ProgressTaskInfo> m_Pending;

  /**
   * \brief Last applied sequence number per task, to drop stale snapshots.
   *
   * Kept past the end of a task, since a snapshot that overtook the final one
   * is exactly what this rejects. Ordered by task id rather than hashed so
   * that the oldest entries, whose tasks can no longer have anything in
   * flight, are the ones dropped when the map is trimmed.
   */
  QMap<mitk::ProgressTaskId, quint64> m_Sequences;

  /** \brief Tasks whose card the user closed while they were still running. */
  QSet<mitk::ProgressTaskId> m_Dismissed;

  QElapsedTimer m_LastRepaint;
};

#endif
