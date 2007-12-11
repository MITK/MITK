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
#include <qprogressbar.h>
#include <mitkProgressBar.h>
//#include <qmainwindow.h>
#include <qapplication.h>
//#include <itkObjectFactory.h>

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
  this->setPercentageVisible(visible);
}

/**
 *Documentation
 *@brief Adds steps to totalSteps.
 */
void QmitkProgressBar::AddStepsToDo(unsigned int steps)
{
  m_TotalSteps += steps;
  this->setProgress(m_Progress, m_TotalSteps);
  if (m_TotalSteps > 0)
  {
    this->show();
  }
  qApp->processEvents();
}
  
/**
 *Documentation
 *@brief Sets the current amount of progress to current progress + steps.
 *@param: steps the number of steps done since last Progress(int steps) call.
 */
void QmitkProgressBar::Progress(unsigned int steps)
{
  m_Progress += steps;
  this->setProgress(m_Progress);
  this->show();
  
  if (m_Progress >= m_TotalSteps)
    Reset();

  qApp->processEvents();
}


QmitkProgressBar::QmitkProgressBar(QWidget * parent, const char * name, WFlags f)
:QProgressBar(parent, name, f), ProgressBarImplementation()
{
    m_TotalSteps = 0;
    m_Progress = 0;
    this->hide();
    this->SetPercentageVisible(true);
    mitk::ProgressBar::SetImplementationInstance(this);
}

QmitkProgressBar::~QmitkProgressBar()
{
}
