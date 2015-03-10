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


#include "mitkIsoDoseLevel.h"

mitk::IsoDoseLevel::IsoDoseLevel() :
m_DoseValue (0.0),
  m_VisibleIsoLine ( true ),
  m_VisibleColorWash (true)
{
  m_Color.Fill(0);
}

mitk::IsoDoseLevel::IsoDoseLevel( const IsoDoseLevel & other )
{
  if (&other != this)
  {
    this->m_Color = other.m_Color;
    this->m_DoseValue = other.m_DoseValue;
    this->m_VisibleColorWash = other.m_VisibleColorWash;
    this->m_VisibleIsoLine = other.m_VisibleIsoLine;
  }
}

mitk::IsoDoseLevel::IsoDoseLevel(const DoseValueType & value, const ColorType& color, bool visibleIsoLine, bool visibleColorWash):
  m_DoseValue (value),
  m_Color (color),
  m_VisibleIsoLine ( visibleIsoLine ),
  m_VisibleColorWash ( visibleColorWash )
{
};

mitk::IsoDoseLevel::~IsoDoseLevel()
{
}

bool mitk::IsoDoseLevel::operator == ( const IsoDoseLevel& right) const
{
  bool result = this->m_DoseValue == right.m_DoseValue;

  result = result && (this->m_Color == right.m_Color);
  result = result && (this->m_VisibleColorWash == right.m_VisibleColorWash);
  result = result && (this->m_VisibleIsoLine == right.m_VisibleIsoLine);

  return result;
}

bool mitk::IsoDoseLevel::operator <( const IsoDoseLevel& right) const
{
  return this->m_DoseValue < right.m_DoseValue;
}

bool mitk::IsoDoseLevel::operator > ( const IsoDoseLevel& right) const
{
  return this->m_DoseValue > right.m_DoseValue;
}

void mitk::IsoDoseLevel::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os<<indent<< "DoseValue: " << m_DoseValue<< std::endl;
  os<<indent<< "Color: " << m_Color<< std::endl;
  os<<indent<< "VisibleIsoLine: " << m_VisibleIsoLine<< std::endl;
  os<<indent<< "VisibleColorWash: " << m_VisibleColorWash<< std::endl;
};
