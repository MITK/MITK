/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Thu, 14 Jan 2010) $
Version:   $Revision: 21047 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkScalarBar.h"

#include <QPainter>
#include <QPaintEvent>

QmitkScalarBar::QmitkScalarBar(QWidget* parent): 
QWidget( parent, Qt::Tool | Qt::FramelessWindowHint ), m_Alignment(vertical)
{
  m_NumberOfSubDivisions = 7;

  this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  this->SetupGeometry( m_Alignment );
  this->setBackgroundRole(QPalette::Base);
  this->setAttribute( Qt::WA_TranslucentBackground, true );
  this->setAutoFillBackground(false);

  // X11 specific attributes
  this->setAttribute( Qt::WA_X11NetWmWindowTypeUtility, true );

  // mac-specific attributes:
  // making sure overlays are even visible if RenderWindow does not have the focus (not default for Qt::Tool on mac)
  this->setAttribute( Qt::WA_MacAlwaysShowToolWindow, true );
  // testing something
  this->setAttribute( Qt::WA_MacShowFocusRect, false );


  this->resize( 20,60 );
  this->setFixedWidth( 20 );
  this->setFixedHeight( 60 );


  m_Pen = QPen( Qt::red, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
}

QmitkScalarBar::~QmitkScalarBar()
{
}

void QmitkScalarBar::SetupGeometry( alignment align )
{
  m_HorizontalLines.clear();
  switch ( align )
  {
  case vertical :
    { 
      m_VerticalLine = new QLine( QPoint(width()/2,0), QPoint(width()/2,height()) );

      for ( unsigned int i=0; i<m_NumberOfSubDivisions; ++i )
      {
        int y = this->height()/(m_NumberOfSubDivisions-1)*i;
        if ( i==0 )
        {
          // this is the first one -> move y 1 down to have this line completely drawn
          y = 1;
        }
        else if ( i==m_NumberOfSubDivisions-1 )
        {
          // this is the last one -> move y 1 up to have this line completely drawn
          y = this->height() - 1;
        }
        m_HorizontalLines.push_back( new QLine( QPoint(0,y), QPoint(width(),y) ) );
      }
      break;
    }
  case horizontal :
    {
      m_VerticalLine = new QLine( QPoint(0,height()/2), QPoint(width(),height()/2) );

      for ( unsigned int i=0; i<m_NumberOfSubDivisions; ++i )
      {
        int x = this->width()/(m_NumberOfSubDivisions-1)*i;
        if ( i==0 )
        {
          x = 1;
        }
        else if ( i==m_NumberOfSubDivisions-1 )
        {
          x = this->width() - 1;
        }
        m_HorizontalLines.push_back( new QLine( QPoint(x,0), QPoint(x,height()) ) );
      }

      break;
    }
  }
}


void QmitkScalarBar::SetScaleFactor( double scale )
{
  m_ScaleFactor = scale;

  // Adopt the number of small, intersecting lines to the size of the widget.
  if ( this->parentWidget() != NULL && this->parentWidget()->parentWidget() != NULL )
  {
    // If the widget is larger than 80% of the size of the parent -> reduce number by two (must not be smaller than 3)
    if ( this->height() > this->parentWidget()->parentWidget()->height()*0.8 && m_NumberOfSubDivisions > 3 )
    {
      m_NumberOfSubDivisions-=2;
    }
    // If the widget is smaller than 30% of the size of the parent -> increase number by two
    else if ( this->height() < this->parentWidget()->parentWidget()->height()*0.4 && m_NumberOfSubDivisions < 7 )
    {
      m_NumberOfSubDivisions+=2;
    }
  }


  //if ( m_Alignment == vertical )
  {
    this->resize( 20, (m_NumberOfSubDivisions-1)*10/m_ScaleFactor );
    this->setFixedWidth( 20 );
    this->setFixedHeight( (m_NumberOfSubDivisions-1)*10/m_ScaleFactor );
    this->SetupGeometry(m_Alignment);
  }
}

void QmitkScalarBar::SetNumberOfSubdivisions( unsigned int subs )
{
  m_NumberOfSubDivisions = subs;
}

unsigned int QmitkScalarBar::GetNumberOfSubdivisions()
{
  return m_NumberOfSubDivisions;
}


void QmitkScalarBar::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  painter.setPen( m_Pen );
  painter.setBrush( Qt::SolidPattern );
  painter.setRenderHint( QPainter::Antialiasing, true );

  painter.drawLine( m_VerticalLine->p1(), m_VerticalLine->p2() );

  foreach( QLine* line, m_HorizontalLines )
  {
    painter.drawLine( line->p1(), line->p2() );
  }
}


void QmitkScalarBar::SetAlignment( alignment align )
{
  m_Alignment = align;
  this->SetupGeometry( align );
}

void QmitkScalarBar::SetPen( const QPen& pen )
{
  m_Pen = pen;
}