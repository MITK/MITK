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

#include "QmitkTextOverlay.h"

#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkPropertyList.h"

#include <itkCommand.h>

QmitkTextOverlay::QmitkTextOverlay( const char* id )
: QmitkOverlay(id)
, m_ObservedProperty( NULL )
, m_ObserverTag(0)
{
  m_Widget = m_Label = new QLabel();
  QmitkOverlay::AddDropShadow( m_Widget );
}

QmitkTextOverlay::~QmitkTextOverlay()
{
  m_PropertyList->GetProperty( m_Id )->RemoveObserver(m_ObserverTag);
}

void QmitkTextOverlay::GenerateData( mitk::PropertyList::Pointer pl )
{
  if ( pl.IsNull() )
    return;

  m_PropertyList = pl;

  if ( m_PropertyList.IsNotNull() )
  {
    this->SetupCallback( m_PropertyList->GetProperty( m_Id ) );

    this->UpdateFontProperties( pl );
    this->UpdateDisplayedTextFromProperties();
  }
  else
  {
    MITK_ERROR << "invalid propList";
  }

}

void QmitkTextOverlay::UpdateDisplayedTextFromProperties()
{
  std::string text;
  if ( m_PropertyList.IsNull() || !m_PropertyList->GetStringProperty( m_Id, text ) )
  {
    MITK_DEBUG << "Property " << m_Id << " could not be found";
  }
  if ( text != m_Label->text().toStdString() )
  {
    m_Label->setText( text.c_str() );
    m_Label->repaint();
  }
}


void QmitkTextOverlay::UpdateFontProperties( mitk::PropertyList::Pointer pl )
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
  {
    colorProp = mitk::ColorProperty::New( 127.0, 196.0, 232.0 );
  }

  mitk::Color color = colorProp->GetColor();
  palette.setColor( QPalette::Foreground, QColor( color[0],color[1],color[2],255 ) );
  palette.setColor( QPalette::Window, Qt::transparent);
  m_Label->setPalette( palette );

  // get the desired opacity of the overlays
  //mitk::FloatProperty::Pointer opacityProperty =
  //  dynamic_cast<mitk::FloatProperty*>( propertyList->GetProperty( "overlay.opacity" ) );

  //if ( opacityProperty.IsNull() )
  //{
  //  m_Label->setWindowOpacity( 1 );
  //}
  //else
  //{
  //  m_Label->setWindowOpacity( opacityProperty->GetValue() );
  //}

   //set the desired font-size of the overlays
  int fontSize = 0;
  if ( !propertyList->GetIntProperty( "overlay.fontSize", fontSize ) )
  {
    fontSize = 9;
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

  m_Label->setFont( font );
}

void QmitkTextOverlay::SetupCallback( mitk::BaseProperty::Pointer prop )
{
  if ( m_ObservedProperty != prop && m_ObserverTag == 0 )
  {
    if ( prop.IsNotNull() )
    {
      if ( m_ObservedProperty.IsNotNull() )
      {
        m_ObservedProperty->RemoveObserver( m_ObserverTag );
      }

      typedef itk::SimpleMemberCommand< QmitkTextOverlay > MemberCommandType;
      MemberCommandType::Pointer propModifiedCommand;
      propModifiedCommand = MemberCommandType::New();
      propModifiedCommand->SetCallbackFunction( this, &QmitkTextOverlay::UpdateDisplayedTextFromProperties );
      m_ObserverTag = prop->AddObserver( itk::ModifiedEvent(), propModifiedCommand );
    }

    m_ObservedProperty = prop;
  }
  else
  {
    MITK_DEBUG << "invalid property";
  }
}

QSize QmitkTextOverlay::GetNeededSize()
{
  QFont font = m_Label->font();
  QFontMetrics fm(font);

  return fm.size( Qt::TextSingleLine, m_Label->text() );
}

