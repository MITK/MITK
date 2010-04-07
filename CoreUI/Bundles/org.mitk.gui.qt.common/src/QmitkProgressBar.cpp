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
  this->reset();
  this->hide();
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

  connect( this, SIGNAL(SignalAddStepsToDo(unsigned int)), this, SLOT(SlotAddStepsToDo(unsigned int)) );
  connect( this, SIGNAL(SignalProgress(unsigned int)), this, SLOT(SlotProgress(unsigned int)) );
  connect( this, SIGNAL(SignalSetPercentageVisible(bool)), this, SLOT(SlotSetPercentageVisible(bool)) );

  mitk::ProgressBar::SetImplementationInstance(this);
}

QmitkProgressBar::~QmitkProgressBar()
{
}

void QmitkProgressBar::SlotProgress(unsigned int steps)
{
  m_Progress += steps;
  this->setValue(m_Progress);

  if (m_Progress >= m_TotalSteps)
    Reset();
  else
     this->show();

  // Update views if repaint has been requested in the meanwhile
  // (because Qt event loop is not reached while progress bar is updating,
  // unless the application is threaded)
  mitk::RenderingManager::GetInstance()->UpdateCallback();
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

  // Update views if repaint has been requested in the meanwhile
  // (because Qt event loop is not reached while progress bar is updating,
  // unless the application is threaded)
  mitk::RenderingManager::GetInstance()->UpdateCallback();
}

void QmitkProgressBar::SlotSetPercentageVisible(bool visible)
{
  this->setTextVisible(visible);
}


