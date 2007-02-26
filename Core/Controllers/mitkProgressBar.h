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


#ifndef MITKPROGRESSBAR_H
#define MITKPROGRESSBAR_H
#include <itkObject.h>
#include "mitkProgressBarImplementation.h"


namespace mitk {
//##Documentation
//## @brief Sending a message to the applications ProgressBar
//##
//## Holds a GUI dependent ProgressBarImplementation and sends the progress further.
//## nearly equal to itk::OutputWindow, 
//## no Window, but one line of text and a delay for clear. 
//## all mitk-classes use this class to display progress on GUI-ProgressBar.
//## The mainapplication has to set the internal held ProgressBarImplementation with SetInstance(..).
//## @ingroup Interaction
class ProgressBar : public itk::Object
{
public:
  itkTypeMacro(ProgressBar, itk::Object);
  
  //##Documentation
  //## @brief static method to get the GUI dependent ProgressBar-instance 
  //## so the methods DisplayText, etc. can be called
  //## No reference counting, cause of decentral static use!
  static ProgressBarImplementation* GetInstance();

  //##Documentation
  //## @brief Supply a GUI- dependent ProgressBar. Has to be set by the application
  //## to connect the application dependent subclass of mitkProgressBar
  //## if you create an instance, then call ->Delete() on the supplied
  //## instance after setting it.
  static void SetInstance(ProgressBarImplementation* instance);

  //##Documentation
  //## @brief Adds steps to totalSteps.
  static void AddStepsToDo(int steps);
  
  //##Documentation
  //## @brief Sets the current amount of progress to current progress + steps.
  //## @param: steps the number of steps done since last Progress(int steps) call.
  static void Progress(int steps = 1);

  //##Documentation
  //## @brief Sets whether the current progress value is displayed.
  static void SetPercentageVisible (bool visible);

protected:

  //##Documentation
  //## @brief Reset the progress bar. The progress bar "rewinds" and shows no progress.
  static void Reset();
  
  ProgressBar();
  
  virtual ~ProgressBar();
  
  static ProgressBarImplementation* m_Instance;

  static int m_Progress;

  static int m_TotalSteps;
};

}// end namespace mitk
#endif /* define MITKPROGRESSBAR_H */
