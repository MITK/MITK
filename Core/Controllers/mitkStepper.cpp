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


#include "mitkStepper.h"

mitk::Stepper::Stepper() : m_Pos(0), m_Steps(0), m_StepSize(1), m_PosToUnitFactor(1), m_ZeroLine(0)
{

}

mitk::Stepper::~Stepper()
{

}

//##ModelId=3DF8B9410142
void mitk::Stepper::Previous()
{
	if(m_Pos>0)
  {
		--m_Pos;
    Modified();
  }
}

//##ModelId=3DF8B92F01DF
void mitk::Stepper::Last()
{
  if(m_Pos != m_Steps-1)
  {
	  m_Pos = m_Steps-1;
    Modified();
  }
}

//##ModelId=3DF8B91502F8
void mitk::Stepper::First()
{
  if(m_Pos != 0)
  {
  	m_Pos = 0;
    Modified();
  }
}

//##ModelId=3DF8B92703A4
void mitk::Stepper::Next()
{
	if(m_Pos<m_Steps)
  {
		++m_Pos;
    Modified();
  }
}

float mitk::Stepper::ConvertPosToUnit(unsigned int posValue)
{
  if(posValue >= m_Steps)
  {
    if(m_Steps==0)
      posValue = 0;
    else
      posValue = m_Steps-1;
  }
  return posValue*m_PosToUnitFactor-m_ZeroLine;
}

unsigned int mitk::Stepper::ConvertUnitToPos(float unitValue)
{
  unsigned int posValue = (unsigned int)(unitValue/m_PosToUnitFactor+m_ZeroLine+0.5);
  if(posValue >= m_Steps)
  {
    if(m_Steps==0)
      posValue = 0;
    else
      posValue = m_Steps-1;
  }
  return posValue;
}

