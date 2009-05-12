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
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>


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

bool mitk::LevelWindowProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LevWin==m_LevWin;
}

const mitk::LevelWindow & mitk::LevelWindowProperty::GetLevelWindow() const
{
    return m_LevWin;
}

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
  xmlWriter.WriteProperty( "MINRANGE", m_LevWin.GetRangeMin());
  xmlWriter.WriteProperty( "MAXRANGE", m_LevWin.GetRangeMax());
  xmlWriter.WriteProperty( "DEFAULTMINRANGE", m_LevWin.GetDefaultRangeMin());
  xmlWriter.WriteProperty( "DEFAULTMAXRANGE", m_LevWin.GetDefaultRangeMax());
  xmlWriter.WriteProperty( "DEFAULTLEVEL", m_LevWin.GetDefaultLevel());
  xmlWriter.WriteProperty( "DEFAULTWINDOW", m_LevWin.GetDefaultWindow());
  xmlWriter.WriteProperty( "LEVEL", m_LevWin.GetLevel() );
  xmlWriter.WriteProperty( "WINDOW", m_LevWin.GetWindow() );
  return true;
}

bool mitk::LevelWindowProperty::ReadXMLData( XMLReader& xmlReader )
{
  ScalarType minRange(-2048), maxRange(4096);
  if ( xmlReader.GetAttribute( "MINRANGE", minRange ) && xmlReader.GetAttribute( "MAXRANGE", maxRange ))
    m_LevWin.SetRangeMinMax(minRange, maxRange);
  
  ScalarType defaultminrange(-2048), defaultmaxrange(4096);
  if ( xmlReader.GetAttribute( "DEFAULTMINRANGE", defaultminrange ) && xmlReader.GetAttribute( "DEFAULTMAXRANGE", defaultmaxrange ))
    m_LevWin.SetDefaultRangeMinMax(defaultminrange, defaultmaxrange);

  ScalarType defaultlevel(0), defaultwindow(400);
  if ( xmlReader.GetAttribute( "DEFAULTLEVEL", defaultlevel ) && xmlReader.GetAttribute( "DEFAULTWINDOW", defaultwindow ))
    m_LevWin.SetDefaultLevelWindow( defaultlevel, defaultwindow );

  ScalarType level(0), window(400);
  if ( xmlReader.GetAttribute( "LEVEL", level ) && xmlReader.GetAttribute( "WINDOW", window ))
    m_LevWin.SetLevelWindow( level, window );

  return true;
}
