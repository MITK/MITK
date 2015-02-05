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

#include "qclickablelabel.h"

#include <QMouseEvent>

#include <iostream>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QClickableLabel::QClickableLabel( QWidget* parent, Qt::WFlags f )
#else
QClickableLabel::QClickableLabel( QWidget* parent, Qt::WindowFlags f )
#endif
:QLabel(parent, f)
{

}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QClickableLabel::QClickableLabel( const QString& text, QWidget* parent, Qt::WFlags f )
#else
QClickableLabel::QClickableLabel( const QString& text, QWidget* parent, Qt::WindowFlags f )
#endif
:QLabel(text, parent, f)
{

}


QClickableLabel::~QClickableLabel()
{

}

void QClickableLabel::AddHotspot( const QString& name, const QRect position )
{
  m_Hotspots.push_back( position );
  m_HotspotIndexForName.insert( std::make_pair( name, (int)m_Hotspots.size()-1 ) );
  m_HotspotNameForIndex.insert( std::make_pair( (int)m_Hotspots.size()-1, name ) );
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

