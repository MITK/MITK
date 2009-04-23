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


#include "QmitkDataManager.h"

#include "QmitkDataManagerControls.h"

#include <QmitkStdMultiWidget.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkFctMediator.h>

#include "datamanager.xpm"

QmitkDataManager::QmitkDataManager(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) 
  : QmitkFunctionality(parent, name, it), m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget)
{
  SetAvailability(true);
}

QmitkDataManager::~QmitkDataManager()
{
}

QString QmitkDataManager::GetFunctionalityName()
{
  //return "DataManager";
  return name();
  
}

QWidget * QmitkDataManager::CreateMainWidget(QWidget *parent)
{
  if (m_MultiWidget == NULL) 
  {
    return m_MultiWidget = new QmitkStdMultiWidget(parent);
  }
  else
  {
    return NULL;
  }
}


QWidget * QmitkDataManager::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
     m_Controls = new QmitkDataManagerControls(parent);
     m_Controls->SetDataTreeIterator(GetDataTreeIterator());
  }
  return m_Controls;
}

void QmitkDataManager::CreateConnections()
{
  connect(m_Controls, SIGNAL( InitializeStandardViews(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
  connect(m_Controls, SIGNAL( InitializeStandardViews(mitk::DataTreeIteratorBase*) ), m_MultiWidget, SLOT( InitializeStandardViews(mitk::DataTreeIteratorBase*) ));
}

QAction * QmitkDataManager::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Data Manager" ), QPixmap((const char**)datamanager_xpm), tr( "&Data Manager" ), 0, parent, GetFunctionalityName() );
  return action;
}
void QmitkDataManager::Activated()
{
  QmitkFunctionality::Activated();
  assert( m_MultiWidget != NULL );
  m_Controls->UpdateRendererCombo();
}

void QmitkDataManager::initWidgets()
{
}


void QmitkDataManager::update()
{
  m_Controls->SetDataTreeIterator(GetDataTreeIterator());
}


