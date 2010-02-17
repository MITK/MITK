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
#include "QmitkColorPropertyView.h"
#include <QPixmap>
#define ROUND_P(x) ((int)((x)+0.5))

QmitkColorPropertyView::QmitkColorPropertyView( const mitk::ColorProperty* property, QWidget* parent )
: QLabel( parent ),
  PropertyView( property ),
  m_ColorProperty(property)
  //m_SelfCall(false)
{
  setText("  "); // two spaces for some minimun height
  setMinimumSize(15,15);
  PropertyChanged();
  
  m_WidgetPalette = QWidget::palette();
  QWidget::setPalette(m_WidgetPalette);
  QWidget::setAutoFillBackground(true);
}

QmitkColorPropertyView::~QmitkColorPropertyView()
{
}
    
//void QmitkColorPropertyView::unsetPalette()
//{
//  // just ignore calls... this widget is the only one to change its background color
//}

//void QmitkColorPropertyView::setPalette( const QPalette& p)
//{
//  // just ignore calls... this widget is the only one to change its background color
//  if (m_SelfCall) QWidget::setPalette(p);
//}

//void QmitkColorPropertyView::setBackgroundMode( QWidget::BackgroundMode )
//{
//  // just ignore calls... this widget is the only one to change its background color
//}

//void QmitkColorPropertyView::setPaletteBackgroundColor( const QColor & )
//{
//  // just ignore calls... this widget is the only one to change its background color
//}

void QmitkColorPropertyView::PropertyChanged()
{
  if ( m_Property )
    DisplayColor();
}

void QmitkColorPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  m_ColorProperty = NULL;
  //QLabel::setPaletteBackgroundPixmap( QPixmap(no_color_icon_xpm) );
}

void QmitkColorPropertyView::DisplayColor()
{
  const mitk::Color& tmp_col(m_ColorProperty->GetColor());

  QColor color( ROUND_P(tmp_col[0] * 255.0), ROUND_P(tmp_col[1] * 255.0) , ROUND_P(tmp_col[2] * 255.0) );
  //m_SelfCall = true;
  //QWidget::setPaletteBackgroundColor( color );
  //m_SelfCall = false;
  
  m_WidgetPalette.setColor(QPalette::Background, color);
}


