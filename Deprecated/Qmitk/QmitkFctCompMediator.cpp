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



// QmitkFctCompMediator.cpp: implementation of the QmitkFctCompMediator class.
//
//////////////////////////////////////////////////////////////////////

#include "QmitkFctCompMediator.h"
#include <qwidget.h>


QmitkFctCompMediator::QmitkFctCompMediator(QWidget *): m_FunctionalityComponentActionGroup(NULL)
{

}

QmitkFctCompMediator::~QmitkFctCompMediator()
{
  QmitkFunctionalityComponentContainer *functionalityComponent;
  for ( functionalityComponent=qfl.first(); functionalityComponent != 0; functionalityComponent=qfl.next() )
    delete functionalityComponent;
  qfl.clear();
}


void QmitkFctCompMediator::Initialize(QWidget* /*aLayoutTemplate*/)
{

}

bool QmitkFctCompMediator::AddFunctionalityComponent(QmitkFunctionalityComponentContainer * functionalityComponent)
{
  qfl.append(functionalityComponent);

//  QPushButton* funcButton=NULL; 

  //QLayout * menuLayout = m_ButtonMenu->layout();
  //if(menuLayout==NULL)
  //{
  //  menuLayout = new QVBoxLayout(m_ButtonMenu,0,10);
  //  menuLayout->setSpacing( 2 );
  //  menuLayout->setMargin( 11 );
  //}

  //funcButton = new QPushButton(functionality->GetFunctionalityName(), m_ButtonMenu, number);
  //funcButton->setToggleButton(true);
  //menuLayout->addItem(new QWidgetItem(funcButton));

  //if((qfl.count()>1) && (functionalityComponent->IsAvailable()==false))
  //{
  //  //funcButton->setEnabled(false);
  //}
  return true;
}
//QmitkFunctionalityComponentContainer* QmitkFctCompMediator::GetFunctionalityComponentByName(const char * name){}
//int QmitkFctCompMediator::GetActiveFunctionalityComponentId(){}
//QmitkFunctionalityComponentContainer* QmitkFctCompMediator::GetActiveFunctionalityComponent(){}
//QmitkFunctionalityComponentContainer* QmitkFctCompMediator::GetFunctionalityComponentById(int id){}
//unsigned int QmitkFctCompMediator::GetFunctionalityComponentCount(){}

