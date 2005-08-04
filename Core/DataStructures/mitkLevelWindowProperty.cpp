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


#include "mitkLevelWindowProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>


//##ModelId=3ED953090121
mitk::LevelWindowProperty::LevelWindowProperty()
{
}

//##ModelId=3EF198D9012D
mitk::LevelWindowProperty::LevelWindowProperty(const mitk::LevelWindow &levWin)
{
    SetLevelWindow(levWin);
}

//##ModelId=3ED953090122
mitk::LevelWindowProperty::~LevelWindowProperty()
{
}

//##ModelId=3ED953090124
bool mitk::LevelWindowProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LevWin==m_LevWin;
}

//##ModelId=3ED953090133
const mitk::LevelWindow & mitk::LevelWindowProperty::GetLevelWindow() const
{
    return m_LevWin;
}

//##ModelId=3ED953090135
void mitk::LevelWindowProperty::SetLevelWindow(const mitk::LevelWindow &levWin)
{
    if(m_LevWin != levWin)
    {
        m_LevWin = levWin;
        Modified();
    }
}

std::string mitk::LevelWindowProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr << "L:" << m_LevWin.GetLevel() << " W:" << m_LevWin.GetWindow();
  return myStr.str(); 
}


bool mitk::LevelWindowProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  xmlWriter.WriteProperty( "LEVEL", m_LevWin.GetLevel() );
  xmlWriter.WriteProperty( "WINDOW", m_LevWin.GetWindow() );
  return true;
}

bool mitk::LevelWindowProperty::ReadXMLData( XMLReader& xmlReader )
{
  double level, window;
  xmlReader.GetAttribute( "LEVEL", level );
  xmlReader.GetAttribute( "WINDOW", window );
  m_LevWin.SetLevelWindow( level, window );
  return true;
}
