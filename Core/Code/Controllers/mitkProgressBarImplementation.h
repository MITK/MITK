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


#ifndef MITKPROGRESSBARIMPLEMENTATION_H
#define MITKPROGRESSBARIMPLEMENTATION_H
#include <MitkExports.h>

namespace mitk
{

  //##Documentation
  //## @brief GUI indepentent Interface for all Gui depentent implementations of a ProgressBar.
  class MITK_CORE_EXPORT ProgressBarImplementation
  {

  public:

    //##Documentation
    //## @brief Constructor
    ProgressBarImplementation(){};

    //##Documentation
    //## @brief Destructor
    virtual ~ProgressBarImplementation(){};

    //##Documentation
    //## @brief Sets whether the current progress value is displayed.
    virtual void SetPercentageVisible (bool visible) =0;

    //##Documentation
    //## @brief Adds steps to totalSteps.
    virtual void AddStepsToDo(unsigned int steps) =0;

    //##Documentation
    //## @brief Sets the current amount of progress to current progress + steps.
    //## @param steps the number of steps done since last Progress(int steps) call.
    virtual void Progress(unsigned int steps) =0;
  };

}// end namespace mitk

#endif /* define MITKPROGRESSBARIMPLEMENTATION_H */

