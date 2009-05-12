/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "qclickablelabel.h"

#include <QMouseEvent>

#include <iostream>

QClickableLabel::QClickableLabel( QWidget* parent, Qt::WFlags f )
:QLabel(parent, f)
{

}


QClickableLabel::QClickableLabel( const QString& text, QWidget* parent, Qt::WFlags f )
:QLabel(text, parent, f)
{

}


QClickableLabel::~QClickableLabel()
{

}

void QClickableLabel::AddHotspot( const QString& name, const QRect position )
{
  m_Hotspots.push_back( position );
  m_HotspotIndexForName.insert( std::make_pair( name, m_Hotspots.size()-1 ) );
  m_HotspotNameForIndex.insert( std::make_pair( m_Hotspots.size()-1, name ) );
}

    
void QClickableLabel::RemoveHotspot( const QString& name )
{
  NameToIndexMapType::iterator iter = m_HotspotIndexForName.find( name );

  if ( iter != m_HotspotIndexForName.end() )
  {
    RemoveHotspot( iter->second );
  }
}

void QClickableLabel::RemoveHotspot( unsigned int hotspotIndex )
{
  if ( hotspotIndex < m_Hotspots.size() )
  {
    m_Hotspots.erase( m_Hotspots.begin() + hotspotIndex );
    QString name = m_HotspotNameForIndex[hotspotIndex];
    m_HotspotNameForIndex.erase( hotspotIndex );
    m_HotspotIndexForName.erase( name );
  }
}

void QClickableLabel::RemoveAllHotspots()
{
  m_Hotspots.clear();
  m_HotspotIndexForName.clear();
  m_HotspotNameForIndex.clear();
}

void QClickableLabel::mousePressEvent( QMouseEvent* e )
{
  unsigned int index = matchingRect( e->pos() );
  if ( index < m_Hotspots.size() )
  {
    emit mouseReleased( index );
    emit mouseReleased( m_HotspotNameForIndex[index] );
  }
}

void QClickableLabel::mouseReleaseEvent( QMouseEvent* e )
{
  unsigned int index = matchingRect( e->pos() );
  if ( index < m_Hotspots.size() )
  {
    emit mousePressed( index );
    emit mousePressed( m_HotspotNameForIndex[index] );
  }
}

unsigned int QClickableLabel::matchingRect( const QPoint& p )
{
  unsigned int index(0);
  for ( RectVectorType::iterator iter = m_Hotspots.begin();
        iter != m_Hotspots.end();
        ++iter )
  {
    if ( iter->contains(p) )
    {
      return index;
    }
    
    ++index;
  }

  return index;
}

