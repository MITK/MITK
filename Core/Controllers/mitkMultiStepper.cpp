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


#include "mitkMultiStepper.h"
#include "mitkStepper.h"

mitk::MultiStepper::MultiStepper() 
{
}

mitk::MultiStepper::~MultiStepper()
{
}
void mitk::MultiStepper::AddStepper(Stepper::Pointer stepper,unsigned int repeat) {
  m_SubSteppers.insert(stepper);
  m_ScaleFactors.insert(std::make_pair(stepper,repeat));
  UpdateStepCount();
}
void mitk::MultiStepper::RemoveStepper(Stepper::Pointer stepper, unsigned int /* repeat */) {
  m_SubSteppers.erase(stepper);
  m_ScaleFactors.erase(stepper);
  UpdateStepCount();
}
/*void mitk::MultiStepper::Next() {
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    (*it)->Next();
  } 
}
void mitk::MultiStepper::Previous() {
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    (*it)->Previous();
  } 
}
void mitk::MultiStepper::First() {
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    (*it)->First();
  } 
}
void mitk::MultiStepper::Last() {
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    (*it)->Last();
  } 
}*/

void mitk::MultiStepper::SetPos(unsigned int pos) {
  Stepper::SetPos(pos);
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    unsigned int count = (*it)->GetSteps() * m_ScaleFactors[(*it)];
	if ((this->GetSteps() != 0 ) && ((*it)->GetSteps() != 0)) {
		(*it)->SetPos((pos * count / this->GetSteps() ) % (*it)->GetSteps()) ;
	}
  }
};

void mitk::MultiStepper::SetSteps(unsigned int /*steps*/) {
  assert(false); 
};
 
void mitk::MultiStepper::UpdateStepCount() {
  m_Steps=0;
  m_LargestRangeStepper = NULL;
  for (StepperSet::iterator it = m_SubSteppers.begin(); it != m_SubSteppers.end() ; it++ ) {
    unsigned int count = (*it)->GetSteps() * m_ScaleFactors[(*it)];
    if (count > m_Steps) {
       m_Steps = count;
       m_LargestRangeStepper = *it;
    }
  } 
}
