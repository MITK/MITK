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

#include "mitkProgressBar.h"
#include "mitkCallbackFromGUIThread.h"
#include "mitkProgressBarImplementation.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>
#include <itkCommand.h>

namespace mitk
{

  ProgressBarImplementation* ProgressBar::m_Implementation = NULL;
  ProgressBar* ProgressBar::m_Instance = NULL;

  /**
   * Sets the current amount of progress to current progress + steps.
   * @param steps the number of steps done since last Progress(int steps) call.
   */
  void ProgressBar::Progress(unsigned int steps)
  {
    if (m_Implementation != NULL)
    {
      m_Implementation->Progress(steps);
    }
  }

  /**
   * Adds steps to totalSteps.
   */
  void ProgressBar::AddStepsToDo(unsigned int steps)
  {
    if (m_Implementation)
    {
      m_Implementation->AddStepsToDo(steps);
    }
  }

  /**
   * Sets whether the current progress value is displayed.
   */
  void ProgressBar::SetPercentageVisible(bool visible)
  {
    if (m_Implementation != NULL)
    {
      m_Implementation->SetPercentageVisible(visible);
    }
  }

  /**
   * Get the instance of this ProgressBar
   */
  ProgressBar* ProgressBar::GetInstance()
  {
    if (m_Instance == NULL)
    {
      m_Instance = new ProgressBar();
    }

    return m_Instance;
  }

  /**
   * Set an instance of this; application must do this!See Header!
   */
  void ProgressBar::SetImplementationInstance(ProgressBarImplementation* implementation)
  {
    if ( m_Implementation == implementation )
    {
      return;
    }
    m_Implementation = implementation;
  }

  ProgressBar::ProgressBar()
  {
  }

  ProgressBar::~ProgressBar()
  {
  }

}//end namespace mitk

