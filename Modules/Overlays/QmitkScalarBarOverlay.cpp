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

#include "QmitkScalarBarOverlay.h"

#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkPropertyList.h"

#include <itkCommand.h>

#include <QLayout>


QmitkScalarBarOverlay::QmitkScalarBarOverlay( const char* id ) 
:QmitkOverlay(id)
, m_ScalarBar( NULL )
, m_ObserverTag(0)
{
  m_Widget = m_ScalarBar = new QmitkScalarBar();
  QmitkOverlay::AddDropShadow( m_ScalarBar );
}

QmitkScalarBarOverlay::~QmitkScalarBarOverlay()
{
  m_PropertyList->GetProperty( m_Id )->RemoveObserver(m_ObserverTag);
}

void QmitkScalarBarOverlay::GenerateData( mitk::PropertyList::Pointer pl )
{
  if ( pl.IsNull() )
    return;

  m_PropertyList = pl;

  if ( m_PropertyList.IsNotNull() )
  {
    this->SetupCallback( m_PropertyList->GetProperty( m_Id ) );

    this->GetProperties( pl );
    this->SetScaleFactor();
  }
  else
  {
    MITK_DEBUG << "invalid propList";
  }

}

void QmitkScalarBarOverlay::SetScaleFactor()
{
  float scale = 2;
  if ( m_PropertyList.IsNull() || !m_PropertyList->GetFloatProperty( m_Id, scale ) )
  {
    MITK_DEBUG << "Property " << m_Id << " could not be found";
  }
  m_ScalarBar->SetScaleFactor( scale );
}


void QmitkScalarBarOverlay::GetProperties( mitk::PropertyList::Pointer pl )
{
  if ( pl.IsNull() )
    return;

  QPen pen = QPen();


  mitk::PropertyList::Pointer propertyList = pl;
  QPalette palette = QPalette();

  // get the desired color of the textOverlays
  mitk::ColorProperty::Pointer colorProp = 
    dynamic_cast<mitk::ColorProperty*>( propertyList->GetProperty( "overlay.color" ) );

  if ( colorProp.IsNull() )
  {
    MITK_DEBUG << "creating new colorProperty";
    colorProp = mitk::ColorProperty::New( 127.0, 196.0, 232.0 );
  }

  mitk::Color color = colorProp->GetColor();
  pen.setColor( QColor( color[0],color[1],color[2],255 ) );
  pen.setStyle( Qt::SolidLine );
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  m_ScalarBar->SetPen( pen );
}

void QmitkScalarBarOverlay::SetupCallback( mitk::BaseProperty::Pointer prop )
{
  if ( prop.IsNotNull() )
  {
    typedef itk::SimpleMemberCommand< QmitkScalarBarOverlay > MemberCommandType;
    MemberCommandType::Pointer propModifiedCommand;
    propModifiedCommand = MemberCommandType::New();
    propModifiedCommand->SetCallbackFunction( this, &QmitkScalarBarOverlay::SetScaleFactor );
    m_ObserverTag = prop->AddObserver( itk::ModifiedEvent(), propModifiedCommand );
  }
  else
  {
    MITK_DEBUG << "invalid property";
  }
}

