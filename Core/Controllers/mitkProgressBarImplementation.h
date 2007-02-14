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


#ifndef MITKPROGRESSBARIMPLEMENTATION_H
#define MITKPROGRESSBARIMPLEMENTATION_H

namespace mitk {
//##Documentation
//## @brief GUI indepentent Interface for all Gui depentent implementations of a ProgressBar.
class ProgressBarImplementation
{
public:
  //##Documentation
  //## @brief Constructor
  ProgressBarImplementation(){};
  //##Documentation
  //## @brief Destructor
  virtual ~ProgressBarImplementation(){};

  //##Documentation
  //## @brief Sets the total number of steps to totalSteps.
  virtual void SetTotalSteps(int totalSteps) =0;
  //##Documentation
  //## @brief Sets the current amount of progress to progress.
  virtual void SetProgress(int progress) =0;
  //##Documentation
  //## @brief Sets the amount of progress to progress and the total number of steps to totalSteps.
  virtual void SetProgress(int progress, int totalSteps) =0;

  //##Documentation
  //## @brief Reset the progress bar. The progress bar "rewinds" and shows no progress.
  virtual void Reset() =0;

  //##Documentation
  //## @brief Sets whether the current progress value is displayed.
  virtual void SetPercentageVisible (bool visible) =0;
};

}// end namespace mitk
#endif /* define MITKPROGRESSBARIMPLEMENTATION_H */
