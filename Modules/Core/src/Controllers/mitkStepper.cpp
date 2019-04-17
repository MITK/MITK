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

#include "mitkStepper.h"

mitk::Stepper::Stepper()
  : m_Pos(0),
    m_Steps(0),
    m_AutoRepeat(false),
    m_PingPong(false),
    m_InverseDirection(false),
    m_RangeMin(0.0),
    m_RangeMax(-1.0),
    m_RangeValid(false),
    m_HasRange(false),
    m_HasUnitName(false)
{
}

mitk::Stepper::~Stepper()
{
}

void mitk::Stepper::SetRange(ScalarType min, ScalarType max)
{
  m_RangeMin = min;
  m_RangeMax = max;
  m_HasRange = true;
  m_RangeValid = true;
  this->Modified();
}

void mitk::Stepper::InvalidateRange()
{
  m_HasRange = true;
  m_RangeValid = false;
  this->Modified();
}

mitk::ScalarType mitk::Stepper::GetRangeMin() const
{
  return m_RangeMin;
}

mitk::ScalarType mitk::Stepper::GetRangeMax() const
{
  return m_RangeMax;
}

void mitk::Stepper::RemoveRange()
{
  m_HasRange = false;
  this->Modified();
}

bool mitk::Stepper::HasValidRange() const
{
  return (m_HasRange && m_RangeValid);
}

bool mitk::Stepper::HasRange() const
{
  return m_HasRange;
}

void mitk::Stepper::SetUnitName(const char *unitName)
{
  m_UnitName = std::string(unitName);
  m_HasUnitName = true;
  this->Modified();
}

const char *mitk::Stepper::GetUnitName() const
{
  return m_UnitName.c_str();
}

void mitk::Stepper::RemoveUnitName()
{
  m_HasUnitName = false;
  this->Modified();
}

bool mitk::Stepper::HasUnitName() const
{
  return m_HasUnitName;
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

void mitk::Stepper::MoveSlice(int sliceDelta)
{
  int newPosition = this->GetPos() + sliceDelta;
  // if auto repeat is on, increasing continues at the first slice if the last slice was reached and vice versa
  int maxSlices = this->GetSteps();
  if (m_AutoRepeat)
  {
    while (newPosition < 0)
    {
      newPosition += maxSlices;
    }
    while (newPosition >= maxSlices)
    {
      newPosition -= maxSlices;
    }
  }
  else
  {
    // if the new slice is below 0 we still show slice 0
    // due to the stepper using unsigned int we have to do this ourselves
    if (newPosition < 1)
    {
      newPosition = 0;
    }
  }

  this->SetPos(newPosition);
}

void mitk::Stepper::First()
{
  this->SetPos(0);
}

void mitk::Stepper::Last()
{
  this->SetPos(this->GetSteps() - 1);
}
