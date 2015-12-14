/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QMITKPROGRESSBAR_H
#define QMITKPROGRESSBAR_H

#include <MitkQtWidgetsExports.h>

#include <mitkProgressBarImplementation.h>
#include <QProgressBar>
#include <QTimer>

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
  QmitkProgressBar(QWidget * parent = nullptr, const char * name = nullptr);

  //##Documentation
  //##@brief Destructor
  virtual ~QmitkProgressBar();

  //##Documentation
  //## @brief Sets whether the current progress value is displayed.
  virtual void SetPercentageVisible(bool visible) override;

  //##Documentation
  //## @brief Adds steps to totalSteps.
  virtual void AddStepsToDo(unsigned int steps) override;

  //##Documentation
  //## @brief Sets the current amount of progress to current progress + steps.
  //## @param: steps the number of steps done since last Progress(int steps) call.
  virtual void Progress(unsigned int steps) override;

signals:

  void SignalAddStepsToDo(unsigned int steps);
  void SignalProgress(unsigned int steps);
  void SignalSetPercentageVisible(bool visible);

public slots:

  void SlotOnTimeout();

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

  QTimer* m_timer;
  bool m_pulce;
};

#endif /* define QMITKPROGRESSBAR_H */
