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

#include "QmitkViewInitialization.h"
#include "QmitkViewInitializationControls.h"
#include "icon.xpm"

#include <qaction.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkRenderWindowSelector.h>

#include <mitkSliceNavigationController.h>

QmitkViewInitialization::QmitkViewInitialization(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
  : QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkViewInitialization::~QmitkViewInitialization()
{
}

QWidget * QmitkViewInitialization::CreateMainWidget(QWidget * /*parent*/)
{
  return NULL;
}

QWidget * QmitkViewInitialization::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkViewInitializationControls(parent);
  }
  return m_Controls;
}

void QmitkViewInitialization::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->pbApply), SIGNAL(clicked()),(QObject*) this, SLOT(Apply()) );
    connect( (QObject*)(m_Controls->pbReset), SIGNAL(clicked()),(QObject*) this, SLOT(ResetAll()) );
  }
}

QAction * QmitkViewInitialization::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "change view initialization" ), QPixmap((const char**)icon_xpm), tr( "&View Initialization" ), 0, parent, "ViewInitialization" );
  return action;
}

void QmitkViewInitialization::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkViewInitialization::Apply()
{
  mitk::SliceNavigationController::ViewDirection viewDirection;
  switch ( m_Controls->bgOrientation->selectedId() )
  {
    default:
    case 0: 
      viewDirection = mitk::SliceNavigationController::Transversal;
      break;
    case 1: 
      viewDirection = mitk::SliceNavigationController::Frontal;
      break;
    case 2: 
      viewDirection = mitk::SliceNavigationController::Sagittal;
      break;            
  }

  vtkRenderWindow* renderwindow = m_Controls->m_RenderWindowSelector->GetSelectedRenderWindow();
  if(renderwindow != NULL)
  {
    mitk::BaseRenderer::GetInstance(renderwindow)->GetSliceNavigationController()->Update(viewDirection, 
      m_Controls->cbTop->isChecked(), 
      m_Controls->cbFrontSide->isChecked(), 
      m_Controls->cbRotated->isChecked()
    );
    mitk::BaseRenderer::GetInstance(renderwindow)->GetDisplayGeometry()->Fit();
  }
}

void QmitkViewInitialization::ResetAll()
{
  m_MultiWidget->ReInitializeStandardViews();
}
