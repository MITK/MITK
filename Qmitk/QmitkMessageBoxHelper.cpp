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

#include "QmitkMessageBoxHelper.h"    
#include "QmitkUserInputSimulation.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qwidget.h>
#include <qobjectlist.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

#include <iostream>

QmitkMessageBoxHelper::QmitkMessageBoxHelper(QObject* parent, const char* name)
:QObject(parent, name),
 m_WhichButton(0),
 m_FoundWidget(NULL),
 m_NumberOfLooks(0),
 m_MaxNumberOfLooks(10)
{
  QObject::connect( &m_Timer, SIGNAL(timeout()), this, SLOT(CloseMessageBoxesNow()) );
  QObject::connect( &m_LookForDialogTimer, SIGNAL(timeout()), this, SLOT(OnLookForDialogTimeout()) );
}

void QmitkMessageBoxHelper::CloseMessageBoxes(unsigned int whichButton, unsigned int delay)
{
  m_WhichButton = whichButton;
  m_Timer.start( delay, false );
}

void QmitkMessageBoxHelper::CloseMessageBoxesNow()
{
std::cout << "CloseMessageBoxesNow:   New attept to close blocking top-level message boxes" << std::endl;
  //QWidgetList* toplevelWidgets = QApplication::topLevelWidgets();
  qApp->processEvents(); // last deperate try
  QWidgetList* toplevelWidgets = QApplication::allWidgets();
  QWidgetListIt toplevelWidgetsIter(*toplevelWidgets);

std::cout << "CloseMessageBoxesNow:   QApplication reports a list of top-level widgets: " << toplevelWidgets->count() << " widgets" << std::endl;
  QWidget* widget;
  while ( (widget = toplevelWidgetsIter.current()) )
  {
    if (widget->isA("QMessageBox")) 
    {
      //std::cout << "Found a message box! Trying to close it as requested..." << std::endl;
std::cout << "CloseMessageBoxesNow: **  Top-level Widget '" << widget->name() << "' is a QMessageBox. Calling its close() method." << std::endl;
      
      QObjectList* childList = widget->queryList( "QPushButton" );
      QObjectListIt childIter( *childList ); 
std::cout << "CloseMessageBoxesNow: Searching " << widget->name() << " for QPushButton children. Found " << childList->count() << " buttons. Task is to click " << m_WhichButton << ". button." << std::endl;

      QObject* child;
      unsigned int count( childList->count()-1 ); // on first testing, qmessagebox inserts buttons right to left (or something similar), so the first button in this list is the right-most
      while ( (child = childIter.current()) )
      {
        QPushButton* button = static_cast<QPushButton*>(child);
std::cout << "CloseMessageBoxesNow: Button '" << button->name() << "' (label '" << button->text().ascii() << "') gets our index " << count << std::endl;

        if ( count == m_WhichButton )
        {
std::cout << "CloseMessageBoxesNow: This is a very good index (==m_WhichButton), will simulate a click on that button." << std::endl;
          //std::cout << m_WhichButton << ". button is labeled '" << button->text().ascii() << "'. Will be clicked now..." << std::endl;
          QmitkUserInputSimulation::MouseDown   ( button, Qt::LeftButton );
          QmitkUserInputSimulation::MouseRelease( button, Qt::LeftButton );

std::cout << "CloseMessageBoxesNow: Terminating timer of QmitkMessageBoxHelper::CloseMessageBoxes(which,delay)" << std::endl;
          m_Timer.stop();
        }

        --count;
        ++childIter;
      }

      delete childList;
    }

    ++toplevelWidgetsIter;
  }

  delete toplevelWidgets;
}

QWidget* QmitkMessageBoxHelper::FindTopLevelWindow( const char* classname )
{
  QWidgetList* toplevelWidgets = QApplication::topLevelWidgets();
  QWidgetListIt toplevelWidgetsIter(*toplevelWidgets);

  QWidget* widget;
  while ( (widget = toplevelWidgetsIter.current()) )
  {
    //std::cout << "  Found class '" << widget->className() << "'" << std::endl;
    if (widget->isA( classname ))
    {
      return widget;
    }

    ++toplevelWidgetsIter;
  }

  delete toplevelWidgets;

  return NULL;
}

void QmitkMessageBoxHelper::WaitForDialogAndCallback( const char* classname, int maxWaitSeconds )
{
  // start timer for 
  m_NumberOfLooks = 0;
  m_MaxNumberOfLooks = maxWaitSeconds * 2; // * 2 because we look every 1/2 second
  m_ClassName = classname;
  m_LookForDialogTimer.start(500);        // look every 1/2 second (500 ms)
}

void QmitkMessageBoxHelper::StopWaitForDialogAndCallback()
{
  m_LookForDialogTimer.stop();
}

void QmitkMessageBoxHelper::SetFoundDialog( QWidget* widget )
{
  m_FoundWidget = widget;
}

void QmitkMessageBoxHelper::DialogFound()
{
  std::cout << "Found dialog (" << m_FoundWidget << ")" << std::endl;

  emit DialogFound( m_FoundWidget );
}

void QmitkMessageBoxHelper::DialogNotFound()
{
  emit DialogFound( NULL );
}

QWidget* QmitkMessageBoxHelper::FindDialogItem(const char* widgetName, QWidget* parent)
{
  std::cout << "Looking for " << widgetName << " below " << parent->name() << "(" << parent->className() << ")" << std::endl;

  QObjectList* childList = parent->queryList( "QWidget" );
  QObjectListIt childIter( *childList ); 

  QString name(widgetName);

  QObject* child;
  while ( (child = childIter.current()) )
  {
    if ( name == child->name() )
    {
      delete childList;
      return (QWidget*)child;
    }

    ++childIter;
  }
 
  std::cout << "Not found. oh oh ..." << std::endl;
  delete childList;
  return NULL;
}

void QmitkMessageBoxHelper::OnFoundWidgetDestroyed()
{
  SetFoundDialog( NULL );
}

void QmitkMessageBoxHelper::OnLookForDialogTimeout()
{
  std::cout << "look, look" << std::endl;

  ++m_NumberOfLooks;

  if (m_NumberOfLooks < m_MaxNumberOfLooks)
  {
    m_FoundWidget = FindTopLevelWindow( m_ClassName.ascii() );

    if (m_FoundWidget)
    {
      std::cout << "Found dialog after " << (float)m_NumberOfLooks / 2.0  << "s" << std::endl;

      QObject::connect( m_FoundWidget, SIGNAL(destroyed()), this, SLOT(OnFoundWidgetDestroyed()));

      m_LookForDialogTimer.stop();
      DialogFound();
    }
  }
  else
  {
    std::cout << "Could not find dialog" << std::endl;

    m_LookForDialogTimer.stop();
    DialogNotFound();
  }
}


