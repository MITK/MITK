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
#include <QmitkColorPropertyView.h>
#include <qpixmap.h>
#include "no_color_icon.xpm"
#define ROUND_P(x) ((int)((x)+0.5))

QmitkColorPropertyView::QmitkColorPropertyView( const mitk::ColorProperty* property, QWidget* parent, const char* name )
: PropertyView( property ),
  QLabel( parent, name ),
  m_ColorProperty(property)
{
  setText("  "); // two spaces for some minimun height
  DisplayColor();
}

QmitkColorPropertyView::~QmitkColorPropertyView()
{
}

void QmitkColorPropertyView::PropertyChanged()
{
  if ( m_Property )
    DisplayColor();
}

void QmitkColorPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  m_ColorProperty = NULL;
  setPaletteBackgroundPixmap( QPixmap(static_cast<const char**>(no_color_icon_xpm)) );
}

void QmitkColorPropertyView::DisplayColor()
{
  const mitk::Color& tmp_col(m_ColorProperty->GetColor());

  QColor color( ROUND_P(tmp_col[0] * 255.0), ROUND_P(tmp_col[1] * 255.0) , ROUND_P(tmp_col[2] * 255.0) );
  setPaletteBackgroundColor( color );
}


