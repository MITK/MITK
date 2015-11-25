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

#include "QmitkProgressBar.h"

#include "mitkProgressBar.h"
#include "mitkRenderingManager.h"

#include <qprogressbar.h>
#include <qapplication.h>


/**
 * Reset the progress bar. The progress bar "rewinds" and shows no progress.
 */
void QmitkProgressBar::Reset()
{
  if (m_pulce)
  {
    QMetaObject::invokeMethod(m_timer, "stop");
  }

  QMetaObject::invokeMethod(this, "reset");
  QMetaObject::invokeMethod(this, "hide");

  m_TotalSteps = 0;
  m_Progress = 0;
}

/**
 * Sets whether the current progress value is displayed.
 */
void QmitkProgressBar::SetPercentageVisible(bool visible)
{
  emit SignalSetPercentageVisible(visible);
}

/**
 *Documentation
 *@brief Adds steps to totalSteps.
 */
void QmitkProgressBar::AddStepsToDo(unsigned int steps)
{
  emit SignalAddStepsToDo(steps);
}

/**
 * Documentation
 * @brief Sets the current amount of progress to current progress + steps.
 * @param steps the number of steps done since last Progress(int steps) call.
 */
void QmitkProgressBar::Progress(unsigned int steps)
{
  emit SignalProgress(steps);
}


QmitkProgressBar::QmitkProgressBar(QWidget * parent, const char *  /*name*/)
:QProgressBar(parent), ProgressBarImplementation()
{
  m_TotalSteps = 0; m_Progress = 0;
  this->hide();
  this->SetPercentageVisible(true);

  m_pulce = false;

  m_timer = new QTimer();
  connect(m_timer, SIGNAL(timeout()), this, SLOT(SlotOnTimeout()));

  connect( this, SIGNAL(SignalAddStepsToDo(unsigned int)), this, SLOT(SlotAddStepsToDo(unsigned int)) );
  connect( this, SIGNAL(SignalProgress(unsigned int)), this, SLOT(SlotProgress(unsigned int)) );
  connect( this, SIGNAL(SignalSetPercentageVisible(bool)), this, SLOT(SlotSetPercentageVisible(bool)) );

  mitk::ProgressBar::GetInstance()->RegisterImplementationInstance(this);
}

QmitkProgressBar::~QmitkProgressBar()
{
  mitk::ProgressBar::GetInstance()->UnregisterImplementationInstance(this);
}

void QmitkProgressBar::SlotProgress(unsigned int steps)
{
  m_Progress += steps;
  this->setValue(m_Progress);

  if (m_Progress >= m_TotalSteps)
  {
    if (!m_pulce)
    {
      Reset();
    }
    else
    {
      if (m_Progress >= m_TotalSteps)
      {
        m_Progress = 0;
        m_TotalSteps = 100;
      }
    }
  }
  else
  {
     this->show();
  }

  // Update views if repaint has been requested in the meanwhile
  // (because Qt event loop is not reached while progress bar is updating,
  // unless the application is threaded)
  //qApp->processEvents();
  mitk::RenderingManager::GetInstance()->ExecutePendingRequests();
}

void QmitkProgressBar::SlotAddStepsToDo(unsigned int steps)
{
  m_TotalSteps += steps;

  this->setMaximum(m_TotalSteps);
  this->setValue(m_Progress);
  if (m_TotalSteps > 0)
  {
    this->show();
  }
  else
  {
    m_pulce = true;

    setRange(0, 0);
    setValue(0);

    m_timer->start(100);

    m_TotalSteps = 100;

    show();
  }

  QApplication::processEvents();

  // Update views if repaint has been requested in the meanwhile
  // (because Qt event loop is not reached while progress bar is updating,
  // unless the application is threaded)
  mitk::RenderingManager::GetInstance()->ExecutePendingRequests();
}

void QmitkProgressBar::SlotSetPercentageVisible(bool visible)
{
  this->setTextVisible(visible);
}

void QmitkProgressBar::SlotOnTimeout()
{
  int progressPos = value();
  progressPos += 10;

  Progress(progressPos);

  QApplication::processEvents();
}
