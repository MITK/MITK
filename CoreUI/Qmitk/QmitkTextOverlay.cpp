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

#include "QmitkTextOverlay.h"

#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkPropertyList.h"

#include <QLayout>


QmitkTextOverlay::QmitkTextOverlay( const char* id ): 
QmitkOverlay(id), m_Widget( NULL )
{
  m_Widget = new QLabel();
}

QmitkTextOverlay::~QmitkTextOverlay()
{
  m_Widget = NULL;
}

void QmitkTextOverlay::GenerateData( mitk::PropertyList::Pointer pl )
{
  if ( pl.IsNull() )
    return;

  this->GetTextProperties( pl );

  std::string text = "";
  pl->GetStringProperty( m_Id, text );
  m_Widget->setText( text.c_str() );
}


void QmitkTextOverlay::GetTextProperties( mitk::PropertyList::Pointer pl )
{
  if ( pl.IsNull() )
    return;

  mitk::PropertyList::Pointer propertyList = pl;
  QPalette palette = QPalette();
  QFont font = QFont();

  // get the desired color of the textOverlays
  mitk::ColorProperty::Pointer colorProp = 
    dynamic_cast<mitk::ColorProperty*>( propertyList->GetProperty( "overlay.color" ) );

  if ( colorProp.IsNull() )
    colorProp = mitk::ColorProperty::New(0,0,0);

  mitk::Color color = colorProp->GetColor();
  palette.setColor( QPalette::Foreground, QColor( color[0],color[1],color[2],255 ) );
  palette.setColor( QPalette::Window, Qt::transparent);
  m_Widget->setPalette( palette );

  // get the desired opacity of the overlays
  //mitk::FloatProperty::Pointer opacityProperty =
  //  dynamic_cast<mitk::FloatProperty*>( propertyList->GetProperty( "overlay.opacity" ) );

  //if ( opacityProperty.IsNull() )
  //{
  //  m_Widget->setWindowOpacity( 1 );
  //} 
  //else
  //{
  //  m_Widget->setWindowOpacity( opacityProperty->GetValue() );
  //}
  
   //set the desired font-size of the overlays
  int fontSize = 0;
  if ( !propertyList->GetIntProperty( "overlay.fontSize", fontSize ) )
  {
    MITK_INFO << "using default fontSize";
    fontSize = 16;
  } 
  font.setPointSize( fontSize );

  bool useKerning = false;
  if ( !propertyList->GetBoolProperty( "overlay.kerning", useKerning ) )
  {
    useKerning = true;
  }
  font.setKerning( useKerning );

  std::string fontFamily = "";
  if ( !propertyList->GetStringProperty( "overlay.fontFamily", fontFamily ) )
  {
    fontFamily = "Verdana";
  }
  font.setFamily(  QString(fontFamily.c_str()) );

  m_Widget->setFont( font );
  
}

QLabel* QmitkTextOverlay::GetWidget()
{
  return m_Widget;
}


