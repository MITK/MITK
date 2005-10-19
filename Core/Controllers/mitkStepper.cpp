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

mitk::Stepper::Stepper()
: m_Pos(0), m_Steps(0), m_StepSize(1), m_PosToUnitFactor(1), m_ZeroLine(0),
  m_AutoRepeat(false), m_PingPong(false), m_InverseDirection(false)
{
}

mitk::Stepper::~Stepper()
{
}

void mitk::Stepper::Increase()
{
  if (m_Pos < m_Steps - 1)
  {
    ++m_Pos;
    this->Modified();
  }
  else if (m_AutoRepeat)
  {
    if (!m_PingPong)
    {
      m_Pos = 0;
    }
    else
    {
      m_InverseDirection = true;
      if (m_Pos > 0)
      {
        --m_Pos;
      }
    }
    this->Modified();
  }
}

void mitk::Stepper::Decrease()
{
  if (m_Pos > 0)
  {
    --m_Pos;
    this->Modified();
  }
  else if (m_AutoRepeat)
  {
    if (!m_PingPong)
    {
      m_Pos = m_Steps - 1;
    }
    else
    {
      m_InverseDirection = false;
      if (m_Pos < m_Steps - 1)
      {
        ++m_Pos;
      }
    }
    this->Modified();
  }
}

//##ModelId=3DF8B92703A4
void mitk::Stepper::Next()
{
  if (!m_InverseDirection)
  {
    this->Increase();
  }
  else
  {
    this->Decrease();
  }
}

//##ModelId=3DF8B9410142
void mitk::Stepper::Previous()
{
  if (!m_InverseDirection)
  {
    this->Decrease();
  }
  else
  {
    this->Increase();
  }
}

//##ModelId=3DF8B91502F8
void mitk::Stepper::First()
{
  if (m_Pos != 0)
  {
    m_Pos = 0;
    this->Modified();
  }
}

//##ModelId=3DF8B92F01DF
void mitk::Stepper::Last()
{
  if (m_Pos != m_Steps - 1)
  {
    m_Pos = m_Steps - 1;
    this->Modified();
  }
}

float mitk::Stepper::ConvertPosToUnit(unsigned int posValue)
{
  if (posValue >= m_Steps)
  {
    if (m_Steps == 0)
    {
      posValue = 0;
    }
    else
    {
      posValue = m_Steps-1;
    }
  }
  return posValue * m_PosToUnitFactor - m_ZeroLine;
}

unsigned int mitk::Stepper::ConvertUnitToPos(float unitValue)
{
  unsigned int posValue = static_cast< unsigned int >(
    unitValue / m_PosToUnitFactor + m_ZeroLine + 0.5
  );

  if (posValue >= m_Steps)
  {
    if (m_Steps == 0)
    {
      posValue = 0;
    }
    else
    {
      posValue = m_Steps-1;
    }
  }
  return posValue;
}
