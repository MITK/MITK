/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiStepper_h
#define mitkMultiStepper_h

#include "MitkDataTypesExtExports.h"
#include "mitkCommon.h"
#include "mitkStepper.h"
#include "mitkVector.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <set>

namespace mitk
{
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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self) void AddStepper(Stepper::Pointer stepper, unsigned int repeat = 1);
    void RemoveStepper(Stepper::Pointer stepper, unsigned int repeat = 1);
    /*void Next();
    void Previous();
    void First();
    void Last(); */
    // unsigned int GetPos();
    void SetPos(unsigned int pos) override;
    void SetSteps(const unsigned int steps) override;

  protected:
    MultiStepper();
    ~MultiStepper() override;
    typedef std::set<Stepper::Pointer> StepperSet;
    typedef std::map<Stepper::Pointer, unsigned int> ScaleFactorMap;
    StepperSet m_SubSteppers;
    ScaleFactorMap m_ScaleFactors;
    Stepper::Pointer m_LargestRangeStepper;
    void UpdateStepCount();
  };

} // namespace mitk

#endif
