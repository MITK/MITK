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


#ifndef QMITKPROGRESSBAR_H
#define QMITKPROGRESSBAR_H
#include <mitkProgressBarImplementation.h>
#include <mitkCommon.h>
#include <qprogressbar.h>

//##Documentation
//## @brief QT-Toolkit/GUI dependent class that provides the QT's ProgressBar
//##
//## All mitk-classes will call this class for output:
//## mitk::ProgressBar::GetInstance();

class QmitkProgressBar : public mitk::ProgressBarImplementation
{

public:

  mitkClassMacro(QmitkProgressBar, mitk::ProgressBarImplementation);
 
  //##Documentation
  //##@brief Constructor;
  //## holds param instance internaly and connects this to the mitkProgressBar
  QmitkProgressBar(QProgressBar* instance);
  
  //##Documentation
  //##@brief Destructor
  virtual ~QmitkProgressBar();

  //##Documentation
  //## @brief Sets whether the current progress value is displayed.
  virtual void SetPercentageVisible(bool visible);

  //##Documentation
  //## @brief Adds steps to totalSteps.
  virtual void AddStepsToDo(int steps);
  
  //##Documentation
  //## @brief Sets the current amount of progress to current progress + steps.
  //## @param: steps the number of steps done since last Progress(int steps) call.
  virtual void Progress(int steps = 1);
    
private:

  //##Documentation
  //## @brief Reset the progress bar. The progress bar "rewinds" and shows no progress.
  void Reset();

  //static Pointer m_Instance;
  QProgressBar* m_ProgressBar;
  
  int m_TotalSteps;
  
  int m_Progress;

};

#endif /* define QMITKPROGRESSBAR_H */
