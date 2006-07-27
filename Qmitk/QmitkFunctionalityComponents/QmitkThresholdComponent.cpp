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
#include "QmitkThresholdComponent.h"


QmitkThresholdComponent::QmitkThresholdComponent( )
{
  SetAvailability(true);
}

QmitkThresholdComponent::~QmitkThresholdComponent()
{

}

QString QmitkThresholdComponent::GetFunctionalityName()
{
 return m_Name;
}

QAction * QmitkThresholdComponent::CreateAction(QActionGroup* m_FunctionalityComponentActionGroup)
{
    QAction* action;
    action = NULL;
//    action = new QAction( tr( "Simple Example" ), QPixmap((const char**)slicer_xpm), tr( "&Simple Example" ), 0, parent, "simple example" );
    return action;
}


QWidget * QmitkThresholdComponent::CreateMainWidget(QWidgetStack* m_MainStack)
{
  return m_MainStack;
}


QWidget * QmitkThresholdComponent::CreateControlWidget(QWidgetStack* m_ControlStack)
{
  return m_ControlStack;
}


void QmitkThresholdComponent::CreateConnections()
{

}


QString QmitkThresholdComponent::GetFunctionalityComponentName()
{
 return m_Name;
}


void QmitkThresholdComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  //assert( multiWidget != NULL );
  //// init widget 4 as a 3D widget
  //multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);

//  if(m_NavigatorsInitialized)
//  {
//    multiWidget->ReInitializeStandardViews();
//  }
}



