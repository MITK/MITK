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

#include "QmitkScalarBar.h"

#include <QPainter>
#include <QPaintEvent>

QmitkScalarBar::QmitkScalarBar(QWidget* parent)
: QWidget( parent, Qt::Tool | Qt::FramelessWindowHint ), m_Alignment(vertical), m_MainLine(NULL)
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


  this->resize( 10,61 );
  this->setFixedWidth( 10 );
  this->setFixedHeight( 61 );

  m_Pen = QPen( Qt::red, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
}

QmitkScalarBar::~QmitkScalarBar()
{
  CleanUpLines();
}

void QmitkScalarBar::SetupGeometry( alignment align )
{
  this->CleanUpLines();

  switch ( align )
  {
  case vertical :
    {
      //draw subdivision
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
        m_SubDivisionLines.push_back( new QLine( QPoint(0,y), QPoint(width()-2,y) ) );
      }

      //draw mainline
      if ( m_SubDivisionLines.size() > 0 )
      {
        m_MainLine = new QLine( QPoint(width()-1,0), QPoint(width()-1,height()) );
      }
      else
      {
        m_MainLine = new QLine( QPoint(0,0), QPoint(0,0) ); //do not draw the line
      }

      break;
    }
  case horizontal :
    {
      //draw subdivision
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
        m_SubDivisionLines.push_back( new QLine( QPoint(x,0), QPoint(x,height()) ) );
      }

      //draw mainline
      if ( m_SubDivisionLines.size() > 0 )
      {
        m_MainLine = new QLine( QPoint(0,height()/2), QPoint(width(),height()/2) );
      }
      else
      {
        m_MainLine = new QLine( QPoint(0,0), QPoint(0,0) ); //do not draw the line
      }


      break;
    }
  }
}

void QmitkScalarBar::CleanUpLines()
{
  foreach(QLine* line, m_SubDivisionLines)
  {
    delete line; //QLine is not a QObject
    line = NULL;
  }

  m_SubDivisionLines.clear();

  if(m_MainLine != NULL)
  {
    delete m_MainLine;
    m_MainLine = NULL;
  }
}


void QmitkScalarBar::SetScaleFactor( double scale )
{
  m_ScaleFactor = scale;

  // Adopt the number of small, intersecting lines to the size of the widget.
  if ( this->parentWidget() != NULL && this->parentWidget()->parentWidget() != NULL )
  {
    // If the widget is larger than 80% of the size of the parent -> reduce number by two (must not be smaller than 3)
    if ( this->height() > this->parentWidget()->parentWidget()->height()*0.7 && m_NumberOfSubDivisions >= 3 )
    {
      m_NumberOfSubDivisions-=2;
    }
    // If the widget is smaller than 30% of the size of the parent -> increase number by two
    else if ( this->height() < this->parentWidget()->parentWidget()->height()*0.4 && ( m_NumberOfSubDivisions < 7 && m_NumberOfSubDivisions > 0 ) )
    {
      m_NumberOfSubDivisions+=2;
    }

    if ( m_NumberOfSubDivisions == 1 )
    {
      this->resize( 0, 0 );
      this->setFixedWidth( 0 );
      this->setFixedHeight( 0 );
    }
    else
    {
      this->resize( 10, (m_NumberOfSubDivisions-1)*10/m_ScaleFactor );
      this->setFixedWidth( 10 );
      this->setFixedHeight( (m_NumberOfSubDivisions-1)*10/m_ScaleFactor );
      this->SetupGeometry(m_Alignment);
    }

    if ( this->height() > this->parentWidget()->parentWidget()->height()*0.7 && m_NumberOfSubDivisions >= 3 )
      SetScaleFactor( scale );

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
  if ( m_NumberOfSubDivisions > 1 )
  {
    try
    {
      //QPainter shadowPainter( this );
      //shadowPainter.setPen( QPen( QColor(0,0,0,255) ) );
      //shadowPainter.setBrush( Qt::SolidPattern );
      //shadowPainter.setRenderHint( QPainter::Antialiasing, true );

      //shadowPainter.drawLine( m_VerticalLine->p1()+QPoint(1,1), m_VerticalLine->p2()+QPoint(1,1) );
      //foreach( QLine* line, m_HorizontalLines )
      //{
      //  shadowPainter.drawLine( line->p1()+QPoint(1,1), line->p2()+QPoint(1,1) );
      //}


      QPainter painter(this);
      painter.setPen( m_Pen );
      painter.setBrush( Qt::SolidPattern );
      painter.setRenderHint( QPainter::Antialiasing, true );

      painter.drawLine( m_MainLine->p1(), m_MainLine->p2() );

      foreach( QLine* line, m_SubDivisionLines )
      {
        painter.drawLine( line->p1(), line->p2() );
      }
    }
    catch (...)
    {
      MITK_ERROR << "ScalarBar cannot be drawn.";
    }
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
