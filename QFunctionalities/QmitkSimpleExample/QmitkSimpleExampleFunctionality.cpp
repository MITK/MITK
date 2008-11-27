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
#include "QmitkMainTemplate.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkStepperAdapter.h>
#include <mitkMovieGenerator.h>


#include <qpushbutton.h>
#include <qfiledialog.h>

// for stereo setting
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
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), multiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), multiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
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
  m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()
    ->InitializeViews(m_DataTreeIterator.GetPointer());
}

void QmitkSimpleExampleFunctionality::generateMovie()
{
  QmitkRenderWindow* movieRenderWindow = GetMovieRenderWindow();
  //mitk::Stepper::Pointer stepper = multiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice();
  mitk::Stepper::Pointer stepper = movieRenderWindow->GetSliceNavigationController()->GetSlice();
  mitk::MovieGenerator::Pointer movieGenerator = mitk::MovieGenerator::New();
  if (movieGenerator.IsNotNull()) {
    movieGenerator->SetStepper( stepper );
    movieGenerator->SetRenderer( mitk::BaseRenderer::GetInstance(movieRenderWindow->GetRenderWindow()) );
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
  mitk::BaseRenderer::GetInstance(multiWidget->mitkWidget4->GetRenderWindow())->SetMapperID(2);
 
  //if(m_NavigatorsInitialized)
  //{
  //  mitk::RenderingManager::GetInstance()->InitializeViews();
  //}
}

void QmitkSimpleExampleFunctionality::stereoSelectionChanged( int id )
{
  /* From vtkRenderWindow.h tells us about stereo rendering:
  Set/Get what type of stereo rendering to use. CrystalEyes mode uses frame-sequential capabilities available in OpenGL to drive LCD shutter glasses and stereo projectors. RedBlue mode is a simple type of stereo for use with red-blue glasses. Anaglyph mode is a superset of RedBlue mode, but the color output channels can be configured using the AnaglyphColorMask and the color of the original image can be (somewhat maintained using AnaglyphColorSaturation; the default colors for Anaglyph mode is red-cyan. Interlaced stereo  mode produces a composite image where horizontal lines alternate between left and right views. StereoLeft and StereoRight modes choose one or the other stereo view. Dresden mode is yet another stereoscopic interleaving.
  */
 
  vtkRenderWindow * vtkrenderwindow = multiWidget->mitkWidget4->GetRenderWindow();
  
  // note: foreground vtkRenderers (at least the department logo renderer) produce errors in stereoscopic visualization.
  // Therefore, we disable the logo visualization during stereo rendering.
  switch(id)
  {
  case 0:
    vtkrenderwindow->StereoRenderOff();
    break;
  case 1:
    vtkrenderwindow->SetStereoTypeToRedBlue();
    vtkrenderwindow->StereoRenderOn();
    multiWidget->DisableDepartmentLogo();
    break;
  case 2:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    multiWidget->DisableDepartmentLogo();
    break;
  }

  
  mitk::BaseRenderer::GetInstance(multiWidget->mitkWidget4->GetRenderWindow())->SetMapperID(2);
  multiWidget->RequestUpdate();
}

QmitkRenderWindow* QmitkSimpleExampleFunctionality::GetMovieRenderWindow()
{
  //check which RenderWindow should be used to generate the movie, e.g. which button is toggled
   if(controls->GetRenderWindow1Button()->isOn())
   {
    return multiWidget->mitkWidget1;
   }
   else if(controls->GetRenderWindow2Button()->isOn())
   {
     return multiWidget->mitkWidget2;
   }
   else if(controls->GetRenderWindow3Button()->isOn())
   {
     return multiWidget->mitkWidget3;
   }
   else if(controls->GetRenderWindow4Button()->isOn())
   {
     return multiWidget->mitkWidget4;
   }
   else //as default take widget1
   {
     return multiWidget->mitkWidget1;
   }

}
