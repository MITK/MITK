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
 * Sets the total number of steps to totalSteps.
 */
void QmitkProgressBar::SetTotalSteps(int totalSteps)
{
  if (m_ProgressBar != NULL)
  {
    m_ProgressBar->setTotalSteps(totalSteps);
    m_ProgressBar->show();
  }
}

/**
 * Sets the current amount of progress to progress.
 */
void QmitkProgressBar::SetProgress(int progress)
{
  if (m_ProgressBar != NULL)
  {
    m_ProgressBar->setProgress(progress);
    m_ProgressBar->show();
  }
}

/**
 * Sets the amount of progress to progress and the total number of steps to totalSteps.
 */
void QmitkProgressBar::SetProgress(int progress, int totalSteps)
{
  if (m_ProgressBar != NULL)
  {
    m_ProgressBar->setProgress(progress, totalSteps);
    m_ProgressBar->show();
  }
}

/**
 * Reset the progress bar. The progress bar "rewinds" and shows no progress.
 */
void QmitkProgressBar::Reset()
{
  if (m_ProgressBar != NULL)
  {
    m_ProgressBar->reset();
    m_ProgressBar->hide();
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


QmitkProgressBar::QmitkProgressBar(QProgressBar* instance)
:ProgressBarImplementation()
{
    m_ProgressBar = instance;
    m_ProgressBar->hide();
    mitk::ProgressBar::SetInstance(this);
}

QmitkProgressBar::~QmitkProgressBar()
{
}
