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


#ifndef STEPPER_H_HEADER_INCLUDED_C1E77191
#define STEPPER_H_HEADER_INCLUDED_C1E77191

#include "mitkCommon.h"
#include "mitkVector.h"
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
class Stepper : public itk::Object
{
public:
  mitkClassMacro(Stepper, itk::Object);
  itkNewMacro(Self);

  itkGetMacro(Pos, unsigned int);
  void SetPos(unsigned int pos) { // copied from itkMacro.h, itkSetClampMacro(...)
    if (this->m_Pos != (pos > m_Steps-1 ? m_Steps-1 : pos)) {
      this->m_Pos = pos > m_Steps-1 ? m_Steps-1 : pos ;
      this->Modified();
    }
  }

  itkGetMacro(Steps, unsigned int);
  itkSetMacro(Steps, unsigned int);

  itkGetMacro(StepSize, unsigned int);
  void SetStepSize(unsigned int stepSize) { // copied from itkMacro.h, itkSetClampMacro(...)
    if (this->m_StepSize != (stepSize > m_Steps-1 ? m_Steps-1 : stepSize)) {
      this->m_StepSize = stepSize > m_Steps-1 ? m_Steps-1 : stepSize;
      this->Modified();
    } 
  }

  virtual float ConvertPosToUnit(unsigned int posValue);
  virtual unsigned int ConvertUnitToPos(float unitValue);

  itkGetMacro(PosToUnitFactor, float);
  itkSetMacro(PosToUnitFactor, float);

  itkGetMacro(ZeroLine, float);
  itkSetMacro(ZeroLine, float);

  itkGetStringMacro(UnitName);
  itkSetStringMacro(UnitName);

  itkGetMacro(AutoRepeat, bool);
  itkSetMacro(AutoRepeat, bool);
  itkBooleanMacro(AutoRepeat);

  //## Causes the stepper to shift direction when the boundary is reached
  itkSetMacro(PingPong, bool);
  itkGetMacro(PingPong, bool);
  itkBooleanMacro(PingPong);

  //## If set to true, the Next() decreases the stepper and Previous()
  //## decreases it
  itkSetMacro(InverseDirection, bool);
  itkGetMacro(InverseDirection, bool);
  itkBooleanMacro(InverseDirection);

  //##ModelId=3DF8B92703A4
  void Next();

  //##ModelId=3DF8B9410142
  void Previous();

  //##ModelId=3DF8B91502F8
  void First();

  //##ModelId=3DF8B92F01DF
  void Last();

protected:
  Stepper();
  virtual ~Stepper();

  void Increase();

  void Decrease();

  //##ModelId=3DD524BD00DC
  unsigned int m_Pos;

  //##ModelId=3DF8F73C0076
  unsigned int m_Steps;

  //##ModelId=3DF8F74101AE
  unsigned int m_StepSize;

  float m_PosToUnitFactor;
  float m_ZeroLine;

  std::string m_UnitName;

  bool m_AutoRepeat;
  
  bool m_PingPong;
  bool m_InverseDirection;

private:

};

} // namespace mitk



#endif /* STEPPER_H_HEADER_INCLUDED_C1E77191 */
