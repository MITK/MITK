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


#include "mitkLevelWindowProperty.h"


mitk::LevelWindowProperty::LevelWindowProperty()
{
}

mitk::LevelWindowProperty::LevelWindowProperty(const mitk::LevelWindow &levWin)
{
    SetLevelWindow(levWin);
}

mitk::LevelWindowProperty::~LevelWindowProperty()
{
}

bool mitk::LevelWindowProperty::IsEqual(const BaseProperty& property) const
{
    return this->m_LevWin == static_cast<const Self&>(property).m_LevWin;
}

bool mitk::LevelWindowProperty::Assign(const BaseProperty& property)
{
  this->m_LevWin = static_cast<const Self&>(property).m_LevWin;
  return true;
}

const mitk::LevelWindow & mitk::LevelWindowProperty::GetLevelWindow() const
{
    return m_LevWin;
}

const mitk::LevelWindow & mitk::LevelWindowProperty::GetValue() const
{
    return GetLevelWindow();
}

void mitk::LevelWindowProperty::SetLevelWindow(const mitk::LevelWindow &levWin)
{
    if(m_LevWin != levWin)
    {
        m_LevWin = levWin;
        Modified();
    }
}

void mitk::LevelWindowProperty::SetValue(const ValueType& levWin)
{
  SetLevelWindow(levWin);
}

std::string mitk::LevelWindowProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr << "L:" << m_LevWin.GetLevel() << " W:" << m_LevWin.GetWindow();
  return myStr.str(); 
}
