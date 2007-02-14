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


#include "mitkProgressBar.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>


namespace mitk {

ProgressBarImplementation *ProgressBar::m_Instance = NULL;

/**
 * Sets the total number of steps to totalSteps.
 */
void ProgressBar::SetTotalSteps(int totalSteps)
{
    if (m_Instance != NULL)
      m_Instance->SetTotalSteps(totalSteps);
}

/**
 * Sets the current amount of progress to progress.
 */
void ProgressBar::SetProgress(int progress)
{
  if (m_Instance != NULL)
    m_Instance->SetProgress(progress);
}

/**
 * Sets the amount of progress to progress and the total number of steps to totalSteps.
 */
void ProgressBar::SetProgress(int progress, int totalSteps)
{
  if (m_Instance != NULL)
    m_Instance->SetProgress(progress, totalSteps);
}

/**
 * Reset the progress bar. The progress bar "rewinds" and shows no progress.
 */
void ProgressBar::Reset()
{
  if ( m_Instance != NULL)
    m_Instance->Reset();
}

/**
 * Sets whether the current progress value is displayed.
 */
void ProgressBar::SetPercentageVisible(bool visible)
{
  if (m_Instance != NULL)
  {
    m_Instance->SetPercentageVisible(visible);
  }
}

/**
 * Get the instance of this ProgressBar
 */
ProgressBarImplementation* ProgressBar::GetInstance()
{
  if (m_Instance == NULL)//if not set, then send a errormessage on OutputWindow
  {
    (itk::OutputWindow::GetInstance())->DisplayErrorText("No instance of mitk::ProgressBar! Check Applikation!");
    //return NULL; m_Instance then is null, so this is redundant!
  }
  
  return m_Instance;
}

/**
 * Set an instance of this; application must do this!See Header!
 */
void ProgressBar::SetInstance(ProgressBarImplementation* instance)
{
  if ( m_Instance == instance )
  {
    return;
  }
  m_Instance = instance;
}

ProgressBar::ProgressBar()
{
}

ProgressBar::~ProgressBar()
{
}

}//end namespace mitk
