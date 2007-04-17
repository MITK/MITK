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
#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qmessagebox.h>

#include <stdlib.h>
#include <iostream>

QmitkFunctionalityTesting::QmitkFunctionalityTesting( QmitkFctMediator* qfm, QObject * parent, const char * name ) 
  : QObject(parent, name), m_QmitkFctMediator(qfm)
{
  QObject::connect( &m_ActivateTimer, SIGNAL(timeout()), this, SLOT(ActivateNextFunctionality()) );
  QObject::connect( &m_CloseMessagesTimer, SIGNAL(timeout()), this, SLOT(CloseFirstMessageBox()) );
}

QmitkFunctionalityTesting::~QmitkFunctionalityTesting()
{

}

void QmitkFunctionalityTesting::CloseFirstMessageBox() {
    bool boxClosed = false;
    QWidgetList* topWidgets = QApplication::topLevelWidgets();
    QWidgetListIt topWidgetsIt(*topWidgets);
    QWidget* widget;
    while ( ( widget = topWidgetsIt.current()) != 0 ) {
       ++topWidgetsIt;
       if (widget->isA("QMessageBox")) {
        std::cout << "Found a toplevel message box! Give it a parent! Closing it ..." << std::endl;
         ((QMessageBox*)widget)->close();
         boxClosed=true;
         break;
       }
    QObjectList *l = widget->queryList( "QMessageBox" );
    QObjectListIt it( *l ); 
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        std::cout << "Found a message box! Closing it ..." << std::endl;
        ((QMessageBox*)obj)->close();
        boxClosed = true;
        break;
    }
    delete l; // delete the list, not the objects
    if (boxClosed) {
      break;
    }  
  }
  delete topWidgets;
  if (boxClosed) {
    // let everything redraw and call self
    m_CloseMessagesTimer.start(5000,true); 
  } else {
    std::cout << "No message box closed" << std::endl; 
  }
}


void QmitkFunctionalityTesting::ActivateNextFunctionality()
{
  std::cout<<"[PASSED]"<<std::endl;
  int nextId = m_QmitkFctMediator->GetActiveFunctionalityId()+1;
  QmitkFunctionality * nextFunctionality = m_QmitkFctMediator->GetFunctionalityById(nextId);
  if(nextFunctionality != NULL)
  {
    std::cout << "Testing Functionality \"" << nextFunctionality->className() <<"\" "<< std::flush;
    m_CloseMessagesTimer.start(10000,true); // close message boxes if RaiseFunctionality doesn't return
    m_QmitkFctMediator->RaiseFunctionality(nextId);
    m_CloseMessagesTimer.stop();
    m_ActivateTimer.start(5000,true); // after redraw activate next
  }
  else
  {
    qApp->quit();
  }
}

int StartQmitkFunctionalityTesting(QmitkFctMediator* qfm)
{
  QmitkFunctionalityTesting *testing = new QmitkFunctionalityTesting(qfm);

  QTimer::singleShot(2000,testing,SLOT(ActivateNextFunctionality())); // 2 seconds single-shot timer
  testing->m_CloseMessagesTimer.start(10000,true); // close message boxes if RaiseFunctionality doesn't return

  std::cout << "Starting QmitkFunctionalityTesting ... " << std::endl;
  if (qfm->GetActiveFunctionality()) {
    std::cout << "Testing Functionality \"" << qfm->GetActiveFunctionality()->className() <<"\": "<< std::flush;
  } 
  else
  { 
    std::cout << "No active functionality yet ..." << std::endl << std::flush;
  }
  qApp->exec();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
