/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkOverlayController.h"

#include "QmitkRenderWindow.h"
#include "QmitkOverlay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <algorithm>


QmitkOverlayController::QmitkOverlayController( QmitkRenderWindow* rw, mitk::PropertyList* pl )
: QObject(), m_RenderWindow( rw ), m_PropertyList( pl )
{
  this->InitializeOverlayLayout();
  this->AdjustOverlayPosition();
  this->SetOverlayVisibility( true );

  if ( m_PropertyList.IsNull() )
    m_PropertyList = mitk::PropertyList::New();

}

QmitkOverlayController::~QmitkOverlayController()
{
}

void QmitkOverlayController::InitializeOverlayLayout()
{
  
  this->InitializeWidget( QmitkOverlay::top_Left );
  this->InitializeWidget( QmitkOverlay::top_Center );
  this->InitializeWidget( QmitkOverlay::top_Right );
  this->InitializeWidget( QmitkOverlay::middle_Left );
  this->InitializeWidget( QmitkOverlay::middle_Right );
  this->InitializeWidget( QmitkOverlay::bottom_Left );
  this->InitializeWidget( QmitkOverlay::bottom_Center );
  this->InitializeWidget( QmitkOverlay::bottom_Right );

}

void QmitkOverlayController::InitializeWidget( QmitkOverlay::DisplayPosition pos )
{
  m_PositionedOverlays[ pos ] = new QWidget( m_RenderWindow, Qt::Tool | Qt::FramelessWindowHint );
  m_PositionedOverlays[ pos ]->setAutoFillBackground(false);
  m_PositionedOverlays[ pos ]->setAttribute( Qt::WA_TranslucentBackground, true );
  //m_PositionedOverlays[ pos ]->setAttribute( Qt::WA_MacAlwaysShowToolWindow, true );
  //m_PositionedOverlays[ pos ]->setAttribute( Qt::WA_MacShowFocusRect, false );
  m_PositionedOverlays[ pos ]->setFocusPolicy( Qt::NoFocus );
  QPalette p = QPalette();
  p.setColor( QPalette::Window, Qt::transparent );
  m_PositionedOverlays[ pos ]->setPalette( p );


  switch ( pos )
  {
  case QmitkOverlay::top_Left : 
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::TopToBottom );
      layout->setAlignment( Qt::AlignLeft );
      break;
    }
  case QmitkOverlay::top_Center : 
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::TopToBottom );
      layout->setAlignment( Qt::AlignCenter );
      layout->setAlignment( Qt::AlignLeft );
      break;
    }
  case QmitkOverlay::top_Right :
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::TopToBottom );
      layout->setAlignment( Qt::AlignRight );
      break;
    }
  case QmitkOverlay::middle_Left : 
    {
      QHBoxLayout* layout = new QHBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::LeftToRight );
      layout->setAlignment( Qt::AlignLeft );
      break;

    }
  case QmitkOverlay::middle_Right : 
    {
      QHBoxLayout* layout = new QHBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::RightToLeft );
      layout->setAlignment( Qt::AlignRight );
      break;
    }
  case QmitkOverlay::bottom_Left : 
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::BottomToTop );
      layout->setAlignment( Qt::AlignLeft );
      break;
    }
  case QmitkOverlay::bottom_Center : 
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::BottomToTop );
      layout->setAlignment( Qt::AlignCenter );
      break;
    }
  case QmitkOverlay::bottom_Right :
    {
      QVBoxLayout* layout = new QVBoxLayout( m_PositionedOverlays[ pos ] );
      layout->setDirection( QBoxLayout::BottomToTop );
      layout->setAlignment( Qt::AlignRight );
      break;
    }
  }
}

void QmitkOverlayController::AdjustOverlayPosition()
{

  if ( m_PositionedOverlays[ QmitkOverlay::top_Left ]->isVisible() )
  {
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint(0,0) );
    m_PositionedOverlays[ QmitkOverlay::top_Left ]->move( pos.x(), pos.y() );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::top_Center ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::top_Center ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint( m_RenderWindow->size().width()/2, 0 ) ) ;
    widget->move( pos.x() - widget->size().width()/2, pos.y() );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::top_Right ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::top_Right ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint( m_RenderWindow->size().width(), 0 ) ) ;
    widget->move( pos.x() - widget->size().width(), pos.y() );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::middle_Left ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::middle_Left ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint( 0, m_RenderWindow->size().height()/2 ) ) ;
    widget->move( pos.x(), pos.y() - widget->size().height()/2 );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::middle_Right ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::middle_Right ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint( m_RenderWindow->size().width(), m_RenderWindow->size().height()/2 ) ) ;
    widget->move( pos.x() - widget->size().width(), pos.y() - widget->size().height()/2 );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::bottom_Left ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::bottom_Left ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint( 0, m_RenderWindow->size().height() ) ) ;
    widget->move( pos.x(), pos.y() - widget->size().height() );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::bottom_Center ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::bottom_Center ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint(  m_RenderWindow->size().width()/2, m_RenderWindow->size().height() ) ) ;
    widget->move( pos.x() - widget->size().width()/2, pos.y() - widget->size().height() );
  }

  if ( m_PositionedOverlays[ QmitkOverlay::bottom_Right ]->isVisible() )
  {
    QWidget* widget = m_PositionedOverlays[ QmitkOverlay::bottom_Right ];
    QPoint pos = m_RenderWindow->mapToGlobal( QPoint(  m_RenderWindow->size().width(), m_RenderWindow->size().height() ) ) ;
    widget->move( pos.x() - widget->size().width(), pos.y() - widget->size().height() );
  }

}


void QmitkOverlayController::SetOverlayVisibility( bool visible )
{
  foreach( QmitkOverlay* overlay, m_AllOverlays )
  {
    if ( visible )
    {
      MITK_INFO << "Setting visible";
      overlay->GetWidget()->show();
    }
    else
    {
      MITK_INFO << "hiding";
      overlay->GetWidget()->hide();
    }
  }
}


void QmitkOverlayController::AddOverlay( QmitkOverlay* overlay )
{
  if ( overlay != NULL )
  {
    QmitkOverlay::DisplayPosition pos = overlay->GetPosition();
    unsigned int layer = overlay->GetLayer();

    m_PropertyList->ConcatenatePropertyList( m_RenderWindow->GetRenderer()->GetRenderingManager()->GetPropertyList(), false );

    overlay->GenerateData( m_PropertyList );

    overlay->GetWidget()->setParent( m_PositionedOverlays[ pos ] );

    int stackLayer = dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ pos ]->layout() )->isEmpty() ? 0 : layer;
  
    switch ( pos )
    {
      case QmitkOverlay::top_Left : {}
      case QmitkOverlay::middle_Left : {}
      case QmitkOverlay::bottom_Left : 
        {
          dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ pos ]->layout() )->insertWidget( overlay->GetLayer(), overlay->GetWidget(), stackLayer, Qt::AlignLeft );    
          break;
        }
      case QmitkOverlay::top_Center : {}
      case QmitkOverlay::bottom_Center :
        {
          dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ pos ]->layout() )->insertWidget( overlay->GetLayer(), overlay->GetWidget(), stackLayer, Qt::AlignCenter );
          break;
        }
      case QmitkOverlay::top_Right : {}
      case QmitkOverlay::middle_Right : {}
      case QmitkOverlay::bottom_Right : 
        {
          dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ pos ]->layout() )->insertWidget( overlay->GetLayer(), overlay->GetWidget(), stackLayer, Qt::AlignRight );    
          break;
        }
    }

    m_PositionedOverlays[ pos ]->show();
    this->AdjustOverlayPosition();
  }
}



void QmitkOverlayController::AlignOverlays()
{
  //MITK_INFO << "mitkOverlayController::AlignOverlays";

  //OverlayVector::iterator overlayIter;

  //for ( overlayIter=m_AllOverlays.begin(); overlayIter!=m_AllOverlays.end(); overlayIter++ )
  //{
  //  int stackLayer = dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ (*overlayIter)->GetPosition() ]->layout() )->isEmpty() ? 0 : layer;
  //  dynamic_cast<QBoxLayout*>( m_PositionedOverlays[ (*overlayIter)->GetPosition() ]->layout() )->addWidget( (*overlayIter)->GetWidget(), stackLayer, Qt::AlignLeft );
  //}
}