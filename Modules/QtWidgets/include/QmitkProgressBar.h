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
 * \brief QT-Toolkit/GUI dependent class that provides the QT's ProgressBar
 *
 * All mitk-classes will call this class for output:
 * mitk::ProgressBar::GetInstance();
 */
class MITKQTWIDGETS_EXPORT QmitkProgressBar : public QProgressBar, public mitk::ProgressBarImplementation
{
  Q_OBJECT

public:
  //##Documentation
  //##@brief Constructor;
  //## holds param instance internally and connects this to the mitkProgressBar
  QmitkProgressBar(QWidget *parent = nullptr, const char *name = nullptr);

  //##Documentation
  //##@brief Destructor
  ~QmitkProgressBar() override;

  //##Documentation
  //## @brief Sets whether the current progress value is displayed.
  void SetPercentageVisible(bool visible) override;

  //##Documentation
  //## @brief Adds steps to totalSteps.
  void AddStepsToDo(unsigned int steps) override;

  //##Documentation
  //## @brief Sets the current amount of progress to current progress + steps.
  //## @param steps the number of steps done since last Progress(int steps) call.
  void Progress(unsigned int steps) override;

signals:

  void SignalAddStepsToDo(unsigned int steps);
  void SignalProgress(unsigned int steps);
  void SignalSetPercentageVisible(bool visible);

protected slots:

  virtual void SlotAddStepsToDo(unsigned int steps);
  virtual void SlotProgress(unsigned int steps);
  virtual void SlotSetPercentageVisible(bool visible);

private:
  //##Documentation
  //## @brief Reset the progress bar. The progress bar "rewinds" and shows no progress.
  void Reset() override;

  unsigned int m_TotalSteps;

  unsigned int m_Progress;
};

#endif
