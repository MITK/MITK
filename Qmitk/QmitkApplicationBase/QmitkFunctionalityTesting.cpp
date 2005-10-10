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

#include <QmitkFunctionalityTesting.h>

#include <QmitkFctMediator.h>

#include <qapplication.h>
#include <qtimer.h>

#include <stdlib.h>
#include <iostream>

QmitkFunctionalityTesting::QmitkFunctionalityTesting( QmitkFctMediator* qfm, QObject * parent, const char * name ) 
  : QObject(parent, name), m_QmitkFctMediator(qfm)
{
  m_Timer = new QTimer( this );
  QObject::connect( m_Timer, SIGNAL(timeout()), this, SLOT(ActivateNextFunctionality()) );
}

QmitkFunctionalityTesting::~QmitkFunctionalityTesting()
{

}

void QmitkFunctionalityTesting::ActivateNextFunctionality()
{
  std::cout<<"[PASSED]"<<std::endl;
  int nextId = m_QmitkFctMediator->GetActiveFunctionalityId()+1;
  QmitkFunctionality * nextFunctionality = m_QmitkFctMediator->GetFunctionalityById(nextId);
  if(nextFunctionality != NULL)
  {
    std::cout << "Testing Functionality \"" << nextFunctionality->className() <<"\" "<< std::flush;
    m_Timer->start( 1000, TRUE ); // 2 seconds single-shot timer
    m_QmitkFctMediator->RaiseFunctionality(nextId);
  }
  else
  {
    qApp->quit();
  }
}

int StartQmitkFunctionalityTesting(QmitkFctMediator* qfm)
{
  QmitkFunctionalityTesting *testing = new QmitkFunctionalityTesting(qfm);

  //QtTestingClass *qttestingclass = new QtTestingClass;
  QTimer *timer = new QTimer( testing );
  QObject::connect( timer, SIGNAL(timeout()), testing, SLOT(ActivateNextFunctionality()) );
  timer->start( 2000, TRUE ); // 2 seconds single-shot timer

  std::cout << "Starting QmitkFunctionalityTesting ... " << std::endl;
  std::cout << "Testing Functionality \"" << qfm->GetActiveFunctionality()->className() <<"\": "<< std::flush;
  qApp->exec();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
