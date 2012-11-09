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

#include "mitkProgressBar.h"
#include "mitkCallbackFromGUIThread.h"
#include "mitkProgressBarImplementation.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>
#include <itkCommand.h>

#include <algorithm>

namespace mitk
{

  ProgressBar* ProgressBar::m_Instance = NULL;

  /**
   * Sets the current amount of progress to current progress + steps.
   * @param steps the number of steps done since last Progress(int steps) call.
   */
  void ProgressBar::Progress(unsigned int steps)
  {
    if ( !m_Implementations.empty() )
    {
      ProgressBarImplementationsListIterator iter;
      for ( iter = m_Implementations.begin(); iter != m_Implementations.end(); iter++ )
      {
        // update progress for all ProgressBarImplementations
        if ( (*iter) != NULL )
        {
          (*iter)->Progress(steps);
        }
      }
    }
  }

  /**
   * Adds steps to totalSteps.
   */
  void ProgressBar::AddStepsToDo(unsigned int steps)
  {
    if ( !m_Implementations.empty() )
    {
      ProgressBarImplementationsListIterator iter;
      for ( iter = m_Implementations.begin(); iter != m_Implementations.end(); iter++ )
      {
        // set steps to do for all ProgressBarImplementations
        if ( (*iter) != NULL )
        {
          (*iter)->AddStepsToDo(steps);
        }
      }
    }
  }

  /**
   * Sets whether the current progress value is displayed.
   */
  void ProgressBar::SetPercentageVisible(bool visible)
  {
    if ( !m_Implementations.empty() )
    {
      ProgressBarImplementationsListIterator iter;
      for ( iter = m_Implementations.begin(); iter != m_Implementations.end(); iter++ )
      {
        // set percentage visible for all ProgressBarImplementations
        if ( (*iter) != NULL )
        {
          (*iter)->SetPercentageVisible(visible);
        }
      }
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
  void ProgressBar::RegisterImplementationInstance(ProgressBarImplementation* implementation)
  {
    if ( std::find( m_Implementations.begin(), m_Implementations.end(), implementation ) == m_Implementations.end() )
    {
      m_Implementations.push_back( implementation );
    }
  }

  void ProgressBar::UnregisterImplementationInstance(ProgressBarImplementation* implementation)
  {
    ProgressBarImplementationsListIterator iter = std::find( m_Implementations.begin(), m_Implementations.end(), implementation );
    if ( iter != m_Implementations.end() )
    {
      m_Implementations.erase( iter );
    }
  }

  ProgressBar::ProgressBar()
  {
  }

  ProgressBar::~ProgressBar()
  {
  }

}//end namespace mitk

