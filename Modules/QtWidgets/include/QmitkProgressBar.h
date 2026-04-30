/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkProgressBar_h
#define QmitkProgressBar_h

#include <MitkQtWidgetsExports.h>

#include <QProgressBar>
#include <mitkProgressBarImplementation.h>

/**
 * \ingroup QmitkModule
 * \brief Qt implementation of the MITK progress bar interface.
 *
 * This class provides a Qt QProgressBar that implements the
 * mitk::ProgressBarImplementation interface. It registers itself with
 * mitk::ProgressBar so that all MITK classes can report progress through
 * mitk::ProgressBar::GetInstance().
 *
 * Thread safety is achieved by using signal/slot connections: the public
 * interface methods emit signals that are dispatched to the GUI thread.
 *
 * \sa mitk::ProgressBar
 * \sa mitk::ProgressBarImplementation
 */
class MITKQTWIDGETS_EXPORT QmitkProgressBar : public QProgressBar, public mitk::ProgressBarImplementation
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the progress bar and registers it with mitk::ProgressBar.
   * \param[in] parent Optional parent widget.
   * \param[in] name   Optional object name (unused, kept for compatibility).
   */
  QmitkProgressBar(QWidget *parent = nullptr, const char *name = nullptr);

  ~QmitkProgressBar() override;

  /**
   * \brief Sets whether the percentage text is displayed on the progress bar.
   * \param[in] visible True to show percentage, false to hide it.
   */
  void SetPercentageVisible(bool visible) override;

  /**
   * \brief Adds the given number of steps to the total step count.
   * \param[in] steps The number of steps to add.
   */
  void AddStepsToDo(unsigned int steps) override;

  /**
   * \brief Advances the progress by the given number of steps.
   *
   * When the progress reaches the total, the bar is automatically reset.
   *
   * \param[in] steps The number of steps completed since the last call.
   */
  void Progress(unsigned int steps) override;

signals:

  /** \brief Internal signal to dispatch AddStepsToDo to the GUI thread. */
  void SignalAddStepsToDo(unsigned int steps);
  /** \brief Internal signal to dispatch Progress to the GUI thread. */
  void SignalProgress(unsigned int steps);
  /** \brief Internal signal to dispatch SetPercentageVisible to the GUI thread. */
  void SignalSetPercentageVisible(bool visible);

protected slots:

  virtual void SlotAddStepsToDo(unsigned int steps);
  virtual void SlotProgress(unsigned int steps);
  virtual void SlotSetPercentageVisible(bool visible);

private:
  /** \brief Reset the progress bar. The progress bar "rewinds" and shows no progress. */
  void Reset() override;

  unsigned int m_TotalSteps;

  unsigned int m_Progress;
};

#endif
