/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <qprogressbar.h>
#include <mitkProgressBar.h>
#include <qmainwindow.h>
#include <itkObjectFactory.h>

/**
 * Reset the progress bar. The progress bar "rewinds" and shows no progress.
 */
void QmitkProgressBar::Reset()
{
  if (m_ProgressBar != NULL)
  {
    m_ProgressBar->reset();
    m_ProgressBar->hide();
    m_TotalSteps = 0;
    m_Progress = 0;
  }
}

/**
 * Sets whether the current progress value is displayed.
 */
void QmitkProgressBar::SetPercentageVisible(bool visible)
{
  if (m_ProgressBar != NULL)
    m_ProgressBar->setPercentageVisible(visible);
}

/**
 *Documentation
 *@brief Adds steps to totalSteps.
 */
void QmitkProgressBar::AddStepsToDo(int steps)
{
  if (m_ProgressBar != NULL)
  {
    m_TotalSteps += steps;
    m_ProgressBar->setProgress(m_Progress, m_TotalSteps);
    m_ProgressBar->show();
  }
}
  
/**
 *Documentation
 *@brief Sets the current amount of progress to current progress + steps.
 *@param: steps the number of steps done since last Progress(int steps) call.
 */
void QmitkProgressBar::Progress(int steps)
{
  if (m_ProgressBar != NULL)
  {
    m_Progress += steps;
    m_ProgressBar->setProgress(m_Progress);
    m_ProgressBar->show();
  }
  if (m_Progress >= m_TotalSteps)
    Reset();
}


QmitkProgressBar::QmitkProgressBar(QProgressBar* instance)
:ProgressBarImplementation()
{
    m_ProgressBar = instance;
    m_TotalSteps = 0;
    m_Progress = 0;
    m_ProgressBar->hide();
    mitk::ProgressBar::SetImplementationInstance(this);
}

QmitkProgressBar::~QmitkProgressBar()
{
}
