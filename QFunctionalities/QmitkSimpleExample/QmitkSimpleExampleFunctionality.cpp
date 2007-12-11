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
#include <mitkMovieGenerator.h>


#include <qpushbutton.h>
#include <qfiledialog.h>

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
    SetAvailability(true);
}

QmitkSimpleExampleFunctionality::~QmitkSimpleExampleFunctionality()
{
//  delete moveNzoom;
}

QWidget * QmitkSimpleExampleFunctionality::CreateMainWidget(QWidget *parent)
{
    QWidget *result = NULL;

    if (multiWidget == NULL) 
    {
        multiWidget = new QmitkStdMultiWidget(parent);
        result = multiWidget;
    }
    else
    {
        result = NULL;
    }
    return result;
}

QWidget * QmitkSimpleExampleFunctionality::CreateControlWidget(QWidget *parent)
{
    if (controls == NULL)
    {
        controls = new QmitkSimpleExampleControls(parent);        
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), multiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorSagittal(), multiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorFrontal(), multiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorTime(), multiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");
        new QmitkStepperAdapter(controls->getMovieNavigatorTime(), multiWidget->GetTimeNavigationController()->GetTime(), "movieNavigatorTimeFromSimpleExample");
    }
    return controls;
}

void QmitkSimpleExampleFunctionality::CreateConnections()
{
  if ( controls ) 
  {
    connect(controls->getStereoSelect(), SIGNAL(activated(int)), this, SLOT(stereoSelectionChanged(int)) );
    connect(controls->getReInitializeNavigatorsButton(), SIGNAL(clicked()), this, SLOT(initNavigators()) );
    connect(controls->getGenerateMovieButton(), SIGNAL(clicked()), this, SLOT(generateMovie()) );
  }
}

QAction * QmitkSimpleExampleFunctionality::CreateAction(QActionGroup *parent)
{
    QAction* action;
    action = new QAction( tr( "Simple Example" ), QPixmap((const char**)slicer_xpm), tr( "&Simple Example" ), 0, parent, "simple example" );
    return action;
}

void QmitkSimpleExampleFunctionality::initNavigators()
{
  m_NavigatorsInitialized = multiWidget->InitializeStandardViews(m_DataTreeIterator.GetPointer());
}

void QmitkSimpleExampleFunctionality::generateMovie()
{
  mitk::Stepper::Pointer stepper = multiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice();
  mitk::MovieGenerator::Pointer movieGenerator = mitk::MovieGenerator::New();
  if (movieGenerator.IsNotNull()) {
    movieGenerator->SetStepper( stepper );
    movieGenerator->SetRenderer( multiWidget->mitkWidget1->GetRenderer() );
    QString movieFileName = QFileDialog::getSaveFileName( QString::null, "Movie (*.avi)", 0, "movie file dialog", "Choose a file name" );
    if (!movieFileName.isEmpty()) {
      movieGenerator->SetFileName( movieFileName.ascii() );
      movieGenerator->WriteMovie();
    }
  }
}

void QmitkSimpleExampleFunctionality::TreeChanged()
{
}

void QmitkSimpleExampleFunctionality::Activated()
{
  QmitkFunctionality::Activated();
  assert( multiWidget != NULL );
  // init widget 4 as a 3D widget
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);

  //if(m_NavigatorsInitialized)
  //{
  //  multiWidget->ReInitializeStandardViews();
  //}
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
    vtkrenderwindow->SetStereoTypeToRedBlue();
    vtkrenderwindow->StereoRenderOn();
    break;
  case 2:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    break;
  }
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
  multiWidget->mitkWidget4->GetRenderer()->GetRenderWindow()->RequestUpdate();
}
