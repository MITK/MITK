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

mitk::Stepper::Stepper() : m_Pos(0), m_Steps(0), m_StepSize(1), 
                           m_AutoRepeat(false), m_PingPong(false), 
                           m_InverseDirection(false)
{
}

mitk::Stepper::~Stepper()
{
}

void mitk::Stepper::Increase()
{
  if (this->GetPos() < this->GetSteps() - 1)
  {
    this->SetPos(this->GetPos() + 1);
  }
  else if (m_AutoRepeat)
  {
    if (!m_PingPong)
    {
      this->SetPos(0);
    }
    else
    {
      m_InverseDirection = true;
      if (this->GetPos() > 0)
      {
        this->SetPos(this->GetPos() - 1);
      }
    }
  }
}

void mitk::Stepper::Decrease()
{
  if (this->GetPos() > 0)
  {
    this->SetPos(this->GetPos() - 1);
  }
  else if (m_AutoRepeat)
  {
    if (!m_PingPong)
    {
      this->SetPos(this->GetSteps() - 1);
    }
    else
    {
      m_InverseDirection = false;
      if (this->GetPos() < this->GetSteps() - 1)
      {
        this->SetPos(this->GetPos() + 1);
      }
    }
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
    this->SetPos(0);
}

//##ModelId=3DF8B92F01DF
void mitk::Stepper::Last()
{
  this->SetPos(this->GetSteps() - 1);
}

