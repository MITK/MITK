/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkProgressNotification_h
#define QmitkProgressNotification_h

#include <MitkQtWidgetsExports.h>

#include <mitkProgressTaskInfo.h>

#include <QString>
#include <QWidget>

namespace Ui
{
  class QmitkProgressNotification;
}

/**
 * \ingroup QmitkModule
 * \brief Shows the progress of a single mitk::ProgressTask.
 *
 * One card of the QmitkProgressNotificationOverlay stack: the name of the
 * operation, its own progress bar, and a button that either dismisses the card
 * or asks the task to stop, depending on whether the task is cancelable.
 *
 * Cards are driven exclusively by mitk::ProgressTaskInfo snapshots and never
 * query the service themselves.
 *
 * \sa QmitkProgressNotificationOverlay
 * \sa mitk::ProgressTask
 */
class MITKQTWIDGETS_EXPORT QmitkProgressNotification : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkProgressNotification(const mitk::ProgressTaskInfo& info, QWidget* parent = nullptr);
  ~QmitkProgressNotification() override;

  /** \brief Get the id of the task this card shows. */
  mitk::ProgressTaskId GetTaskId() const;

  /** \brief Apply a newer snapshot of the task. */
  void Update(const mitk::ProgressTaskInfo& info);

  /**
   * \brief Show the final state of the task, then fade out.
   *
   * Emits Closed() once the card is ready to be removed. Holding the completed
   * state for a moment is deliberate: a card that vanishes the instant it
   * fills up leaves the user unsure whether the operation ran at all.
   */
  void Finish(const mitk::ProgressTaskInfo& info);

  /** \brief Whether the task this shows has ended. */
  bool IsFinished() const;

signals:
  /** \brief The user asked for the task to stop. */
  void CancelRequested(mitk::ProgressTaskId id);

  /** \brief The card is done and its owner should remove it. */
  void Closed(mitk::ProgressTaskId id);

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  void OnCloseButtonClicked();
  bool ShouldSpin() const;
  void ApplyState();
  void UpdateNameLabel();
  void FadeOut();

  Ui::QmitkProgressNotification* m_Controls;
  mitk::ProgressTaskInfo m_Info;
  QString m_Name;
};

#endif
