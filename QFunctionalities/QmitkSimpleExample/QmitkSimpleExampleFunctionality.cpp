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


#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkSimpleExampleControls.h"

#include <qaction.h>
#include "slicer.xpm"

// for slice-navigation
#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkStepperAdapter.h>
#include "qpushbutton.h"

// for stereo setting
#include <mitkOpenGLRenderer.h>
#include <mitkVtkRenderWindow.h>
#include <vtkRenderWindow.h>

// for zoom/pan
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <mitkDisplayInteractor.h>
#include <mitkInteractionConst.h>

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget), m_NavigatorsInitialized(false)
{
    setAvailability(true);

    moveNzoom = new mitk::DisplayInteractor();
    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    globalInteraction->AddListener(new mitk::DisplayVectorInteractor("moveNzoom", moveNzoom));//sends DisplayCoordinateOperation
    }
}

QmitkSimpleExampleFunctionality::~QmitkSimpleExampleFunctionality()
{
  delete moveNzoom;
}

QWidget * QmitkSimpleExampleFunctionality::createMainWidget(QWidget *parent)
{
    if (multiWidget == NULL) 
    {
        return multiWidget = new QmitkStdMultiWidget(parent);
    }
    else
        return NULL;
}

QWidget * QmitkSimpleExampleFunctionality::createControlWidget(QWidget *parent)
{
    if (controls == NULL)
    {
        controls = new QmitkSimpleExampleControls(parent);        
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), multiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorSagittal(), multiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorFrontal(), multiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorTime(), multiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");

        mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
        if(globalInteraction!=NULL)
        {
          globalInteraction->AddListener(multiWidget->GetMultiplexUpdateController());
        }
    }
    return controls;
}

void QmitkSimpleExampleFunctionality::createConnections()
{
  if ( controls ) 
  {
    connect(controls->getStereoSelect(), SIGNAL(activated(int)), this, SLOT(stereoSelectionChanged(int)) );
    connect(controls->getReInitializeNavigatorsButton(), SIGNAL(clicked()), this, SLOT(initNavigators()) );
  }
}

QAction * QmitkSimpleExampleFunctionality::createAction(QActionGroup *parent)
{
    QAction* action;
    action = new QAction( tr( "Left" ), QPixmap((const char**)slicer_xpm), tr( "&Left" ), CTRL + Key_L, parent, "simple example" );
    return action;
}

void QmitkSimpleExampleFunctionality::initNavigators()
{
  m_NavigatorsInitialized = multiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
}

void QmitkSimpleExampleFunctionality::treeChanged()
{
  if(m_NavigatorsInitialized==false)
    initNavigators();
}

void QmitkSimpleExampleFunctionality::activated()
{
  QmitkFunctionality::activated();
  assert( multiWidget != NULL );
  // init widget 4 as a 3D widget
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);

  if(m_NavigatorsInitialized)
  {
    multiWidget->InitializeStandardViews(NULL);
  }
}

void QmitkSimpleExampleFunctionality::stereoSelectionChanged( int id )
{
  vtkRenderWindow * vtkrenderwindow =
    ((mitk::OpenGLRenderer*)(multiWidget->mitkWidget4->GetRenderer()))->GetVtkRenderWindow();
  switch(id)
  {
  case 0:
    vtkrenderwindow->StereoRenderOff();
    break;
  case 1:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    break;
  }
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
  multiWidget->mitkWidget4->GetRenderer()->GetRenderWindow()->Repaint();
}
