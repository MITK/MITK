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


#ifndef MULTISTEPPER_H_HEADER_INCLUDED
#define MULTISTEPPER_H_HEADER_INCLUDED

#include <set>
#include "MitkDataTypesExtExports.h"
#include "mitkCommon.h"
#include "mitkVector.h"
#include "mitkStepper.h"
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk {

//##Documentation
//## @brief Helper class to step through a list
//##
//## A helper class to step through a list. Does not contain the list, just the
//## position in the list (between 0 and GetSteps()). Provides methods like
//## First (go to the first element), Next (go to the next one), etc.
//## @ingroup NavigationControl
class MITKDATATYPESEXT_EXPORT MultiStepper : public Stepper
{
public:
  mitkClassMacro(MultiStepper, Stepper);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
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
