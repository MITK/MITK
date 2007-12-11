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


#ifndef MULTISTEPPER_H_HEADER_INCLUDED
#define MULTISTEPPER_H_HEADER_INCLUDED

#include <set>
#include "mitkCommon.h"
#include "mitkVector.h"
#include "mitkStepper.h"
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk {

//##ModelId=3DF8BF9A005D
//##Documentation
//## @brief Helper class to step through a list
//##
//## A helper class to step through a list. Does not contain the list, just the
//## position in the list (between 0 and GetSteps()). Provides methods like
//## First (go to the first element), Next (go to the next one), etc.
//## @ingroup NavigationControl
class MultiStepper : public Stepper
{
public:
  mitkClassMacro(MultiStepper, Stepper);
  itkNewMacro(Self);
  void AddStepper(Stepper::Pointer stepper,unsigned int repeat = 1);
  void RemoveStepper(Stepper::Pointer stepper,unsigned int repeat = 1);
  /*void Next();
  void Previous();
  void First();
  void Last(); */
  // unsigned int GetPos();
  void SetPos(unsigned int pos);
  void SetSteps(const unsigned int steps);
   
protected:
  MultiStepper();
  virtual ~MultiStepper();
  typedef std::set<Stepper::Pointer> StepperSet;
  typedef std::map<Stepper::Pointer,unsigned int> ScaleFactorMap;
  StepperSet m_SubSteppers;
  ScaleFactorMap m_ScaleFactors;
  Stepper::Pointer m_LargestRangeStepper;
  void UpdateStepCount();
};

} // namespace mitk



#endif /* MULTISTEPPER_H_HEADER_INCLUDED */
