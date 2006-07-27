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
#include "QmitkFunctionalityComponentContainer.h"


QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer( )
{
  SetAvailability(true);
}

QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{

}

QString QmitkFunctionalityComponentContainer::GetFunctionalityName()
{
 return m_Name;
}

QAction * QmitkFunctionalityComponentContainer::CreateAction(QActionGroup* m_FunctionalityComponentActionGroup)
{
    QAction* action;
    action = NULL;
//    action = new QAction( tr( "Simple Example" ), QPixmap((const char**)slicer_xpm), tr( "&Simple Example" ), 0, parent, "simple example" );
    return action;
}


QWidget * QmitkFunctionalityComponentContainer::CreateMainWidget(QWidgetStack* m_MainStack)
{
  return m_MainStack;
}


QWidget * QmitkFunctionalityComponentContainer::CreateControlWidget(QWidgetStack* m_ControlStack)
{
  return m_ControlStack;
}


void QmitkFunctionalityComponentContainer::CreateConnections()
{

}


QString QmitkFunctionalityComponentContainer::GetFunctionalityComponentName()
{
 return m_Name;
}


void QmitkFunctionalityComponentContainer::Activated()
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

/******************************************************************************************/
void QmitkFunctionalityComponentContainer::CreateSelector()
{}

