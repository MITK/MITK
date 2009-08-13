/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkSimpleExampleView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include <QMessageBox>

#include <mitkMovieGenerator.h>
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateNOT.h"
#include "mitkProperties.h"
#include <QmitkStepperAdapter.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>


#include <vtkRenderWindow.h>
#include "vtkImageWriter.h"
#include "vtkPNGWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkRenderLargeImage.h"

const std::string QmitkSimpleExampleView::VIEW_ID = "org.mitk.views.simpleexample";

QmitkSimpleExampleView::QmitkSimpleExampleView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL),
  m_NavigatorsInitialized(false)
{
}

QmitkSimpleExampleView::~QmitkSimpleExampleView()
{
}

void QmitkSimpleExampleView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkSimpleExampleViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkSimpleExampleView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;

  new QmitkStepperAdapter(m_Controls->sliceNavigatorTransversal, m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");
  new QmitkStepperAdapter(m_Controls->sliceNavigatorSagittal, m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");
  new QmitkStepperAdapter(m_Controls->sliceNavigatorFrontal, m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");
  new QmitkStepperAdapter(m_Controls->sliceNavigatorTime, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample");
  new QmitkStepperAdapter(m_Controls->movieNavigatorTime, m_MultiWidget->GetTimeNavigationController()->GetTime(), "movieNavigatorTimeFromSimpleExample");
}

void QmitkSimpleExampleView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkSimpleExampleView::CreateConnections()
{
  if ( m_Controls )
  {
    connect(m_Controls->stereoSelect, SIGNAL(activated(int)), this, SLOT(stereoSelectionChanged(int)) );
    connect(m_Controls->reInitializeNavigatorsButton, SIGNAL(clicked()), this, SLOT(initNavigators()) );
    connect(m_Controls->genMovieButton, SIGNAL(clicked()), this, SLOT(generateMovie()) );
    connect(m_Controls->m_RenderWindow1Button, SIGNAL(clicked()), this, SLOT(OnRenderWindow1Clicked()) );
    connect(m_Controls->m_RenderWindow2Button, SIGNAL(clicked()), this, SLOT(OnRenderWindow2Clicked()) );
    connect(m_Controls->m_RenderWindow3Button, SIGNAL(clicked()), this, SLOT(OnRenderWindow3Clicked()) );
    connect(m_Controls->m_RenderWindow4Button, SIGNAL(clicked()), this, SLOT(OnRenderWindow4Clicked()) );
    connect(m_Controls->m_TakeScreenshotBtn, SIGNAL(clicked()), this, SLOT(OnTakeScreenshot()) );
    connect(m_Controls->m_TakeHighResScreenShotBtn, SIGNAL(clicked()), this, SLOT(OnTakeHighResolutionScreenshot()) );
  }
}

void QmitkSimpleExampleView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkSimpleExampleView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkSimpleExampleView::initNavigators()
{
  /* get all nodes that have not set "includeInBoundingBox" to false */
  mitk::NodePredicateNOT::Pointer pred = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  /* calculate bounding geometry of these nodes */
  mitk::Geometry3D::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);
  /* initialize the views to the bounding geometry */
  m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  //m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()->InitializeViews(GetDefaultDataStorage());
}

void QmitkSimpleExampleView::generateMovie()
{
  QmitkRenderWindow* movieRenderWindow = GetMovieRenderWindow();
  //mitk::Stepper::Pointer stepper = multiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice();
  mitk::Stepper::Pointer stepper = movieRenderWindow->GetSliceNavigationController()->GetSlice();
  mitk::MovieGenerator::Pointer movieGenerator = mitk::MovieGenerator::New();
  if (movieGenerator.IsNotNull()) {
    movieGenerator->SetStepper( stepper );
    movieGenerator->SetRenderer( mitk::BaseRenderer::GetInstance(movieRenderWindow->GetRenderWindow()) );

    QString movieFileName = QFileDialog::getSaveFileName(0, "Choose a file name", QString(), "Movie (*.avi)");
    if (!movieFileName.isEmpty()) {
      movieGenerator->SetFileName( movieFileName.toStdString().c_str() );
      movieGenerator->WriteMovie();
    }
  }
}

void QmitkSimpleExampleView::stereoSelectionChanged( int id )
{
  /* From vtkRenderWindow.h tells us about stereo rendering:
  Set/Get what type of stereo rendering to use. CrystalEyes mode uses frame-sequential capabilities available in OpenGL to drive LCD shutter glasses and stereo projectors. RedBlue mode is a simple type of stereo for use with red-blue glasses. Anaglyph mode is a superset of RedBlue mode, but the color output channels can be configured using the AnaglyphColorMask and the color of the original image can be (somewhat maintained using AnaglyphColorSaturation; the default colors for Anaglyph mode is red-cyan. Interlaced stereo  mode produces a composite image where horizontal lines alternate between left and right views. StereoLeft and StereoRight modes choose one or the other stereo view. Dresden mode is yet another stereoscopic interleaving.
  */

  vtkRenderWindow * vtkrenderwindow = m_MultiWidget->mitkWidget4->GetRenderWindow();

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
    m_MultiWidget->DisableDepartmentLogo();
    break;
  case 2:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    m_MultiWidget->DisableDepartmentLogo();
    break;
  }


  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow())->SetMapperID(2);
  m_MultiWidget->RequestUpdate();
}

QmitkRenderWindow* QmitkSimpleExampleView::GetMovieRenderWindow()
{
  //check which RenderWindow should be used to generate the movie, e.g. which button is toggled
   if(m_Controls->m_RenderWindow1Button->isChecked())
   {
    return m_MultiWidget->mitkWidget1;
   }
   else if(m_Controls->m_RenderWindow2Button->isChecked())
   {
     return m_MultiWidget->mitkWidget2;
   }
   else if(m_Controls->m_RenderWindow3Button->isChecked())
   {
     return m_MultiWidget->mitkWidget3;
   }
   else if(m_Controls->m_RenderWindow4Button->isChecked())
   {
     return m_MultiWidget->mitkWidget4;
   }
   else //as default take widget1
   {
     return m_MultiWidget->mitkWidget1;
   }

}

void QmitkSimpleExampleView::OnRenderWindow1Clicked()
{
  m_Controls->m_RenderWindow2Button->setChecked(false);
  m_Controls->m_RenderWindow3Button->setChecked(false);
  m_Controls->m_RenderWindow4Button->setChecked(false);
}
void QmitkSimpleExampleView::OnRenderWindow2Clicked()
{
  m_Controls->m_RenderWindow1Button->setChecked(false);
  m_Controls->m_RenderWindow3Button->setChecked(false);
  m_Controls->m_RenderWindow4Button->setChecked(false);
}
void QmitkSimpleExampleView::OnRenderWindow3Clicked()
{
  m_Controls->m_RenderWindow2Button->setChecked(false);
  m_Controls->m_RenderWindow1Button->setChecked(false);
  m_Controls->m_RenderWindow4Button->setChecked(false);
}
void QmitkSimpleExampleView::OnRenderWindow4Clicked()
{
  m_Controls->m_RenderWindow2Button->setChecked(false);
  m_Controls->m_RenderWindow3Button->setChecked(false);
  m_Controls->m_RenderWindow1Button->setChecked(false);
}


void QmitkSimpleExampleView::OnTakeHighResolutionScreenshot()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");

  QmitkRenderWindow* renWin = this->GetMovieRenderWindow();
  if (renWin == NULL)
    return;

  vtkRenderer* renderer = renWin->GetRenderer()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 4, fileName);
}

void QmitkSimpleExampleView::OnTakeScreenshot()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");
  
  QmitkRenderWindow* renWin = this->GetMovieRenderWindow();
  if (renWin == NULL)
    return;

  vtkRenderer* renderer = renWin->GetRenderer()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 1, fileName);
}


void QmitkSimpleExampleView::TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName)
{
  if ((renderer == NULL) ||(magnificationFactor < 1) || fileName.isEmpty())
    return;

  bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
  renderer->GetRenderWindow()->DoubleBufferOff();

  vtkImageWriter* fileWriter;

  QFileInfo fi(fileName);
  QString suffix = fi.suffix();
  if (suffix.compare("png", Qt::CaseInsensitive) == 0)
  {
    fileWriter = vtkPNGWriter::New();
  }
  else  // default is jpeg
  {
    vtkJPEGWriter* w = vtkJPEGWriter::New();
    w->SetQuality(100);
    w->ProgressiveOff();
    fileWriter = w;
  }
  vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
  magnifier->SetInput(renderer);
  magnifier->SetMagnification(magnificationFactor);
  //magnifier->Update();
  fileWriter->SetInput(magnifier->GetOutput());
  fileWriter->SetFileName(fileName.toLatin1());

  // vtkRenderLargeImage has problems with different layers, therefore we have to 
  // temporarily deactivate all other layers.
  // we set the background to white, because it is nicer than black...
  double oldBackground[3];
  renderer->GetBackground(oldBackground);
  double white[] = {1.0, 1.0, 1.0};
  renderer->SetBackground(white);
  m_MultiWidget->DisableColoredRectangles();
  m_MultiWidget->DisableDepartmentLogo();
  m_MultiWidget->DisableGradientBackground();

  fileWriter->Write();  
  fileWriter->Delete();  

  m_MultiWidget->EnableColoredRectangles();
  m_MultiWidget->EnableDepartmentLogo();
  m_MultiWidget->EnableGradientBackground();
  renderer->SetBackground(oldBackground);
  
  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}
