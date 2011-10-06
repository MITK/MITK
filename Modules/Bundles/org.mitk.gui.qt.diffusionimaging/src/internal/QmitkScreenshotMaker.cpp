/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16947 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#include "QmitkScreenshotMaker.h"
//#include "QmitkMovieMakerControls.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"

#include "mitkVtkPropRenderer.h"
#include "mitkGlobalInteraction.h"

#include <iostream>

#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <qaction.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcolor.h>
#include <qcolordialog.h>

#include "qapplication.h"

#include "vtkImageWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkPNGWriter.h"
#include "vtkRenderLargeImage.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"

#include <vtkActor.h>
#include "vtkMitkRenderProp.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkRenderWindowInteractor.h"
#include <qradiobutton.h>

#include "mitkSliceNavigationController.h"
#include "mitkPlanarFigure.h"

QmitkScreenshotMaker::QmitkScreenshotMaker(QObject *parent, const char * /*name*/)

:
  QmitkFunctionality(), m_Controls(NULL),
  m_SelectedNode(0),
  m_BackgroundColor(QColor(0,0,0))
{

  parentWidget = parent;

}

QmitkScreenshotMaker::~QmitkScreenshotMaker()
{
}

void QmitkScreenshotMaker::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*) m_Controls->m_AllViews, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateMultiplanar3DHighresScreenshot()));
    connect((QObject*) m_Controls->m_View1, SIGNAL(clicked()), (QObject*) this, SLOT(View1()));
    connect((QObject*) m_Controls->m_View2, SIGNAL(clicked()), (QObject*) this, SLOT(View2()));
    connect((QObject*) m_Controls->m_View3, SIGNAL(clicked()), (QObject*) this, SLOT(View3()));
    connect((QObject*) m_Controls->m_Shot, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateMultiplanarScreenshots()));
    connect((QObject*) m_Controls->m_BackgroundColor, SIGNAL(clicked()), (QObject*) this, SLOT(SelectBackgroundColor()));

    connect((QObject*) m_Controls->btnScreenshot, SIGNAL(clicked()), this, SLOT(
        GenerateScreenshot()));
    connect((QObject*) m_Controls->m_HRScreenshot, SIGNAL(clicked()), this, SLOT(
        Generate3DHighresScreenshot()));

    // blocking of ui elements during movie generation
    connect((QObject*) this, SIGNAL(StartBlockControls()), (QObject*) this, SLOT(BlockControls()));

    connect((QObject*) this, SIGNAL(EndBlockControls()), (QObject*) this, SLOT(UnBlockControls()));

    connect((QObject*) this, SIGNAL(EndBlockControlsMovieDeactive()), (QObject*) this, SLOT(
        UnBlockControlsMovieDeactive()));


  }
}

void QmitkScreenshotMaker::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkScreenshotMaker::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkScreenshotMaker::GenerateScreenshot()
{
  emit StartBlockControls();

  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");

  vtkRenderer* renderer = mitk::GlobalInteraction::GetInstance()->GetFocus()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 1, fileName);

  emit EndBlockControlsMovieDeactive();
}

void QmitkScreenshotMaker::GenerateMultiplanarScreenshots()
{
  QString fileName = QFileDialog::getExistingDirectory(NULL, "Save screenshots to...", QDir::currentPath());

  emit StartBlockControls();

  mitk::DataNode* n;
  n = GetDataStorage()->GetNamedNode("widget1Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 1,1,1 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  n = GetDataStorage()->GetNamedNode("widget2Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 1,1,1 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  n = GetDataStorage()->GetNamedNode("widget3Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 1,1,1 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  // only works correctly for 3D RenderWindow
  vtkRenderer* renderer = m_MultiWidget->mitkWidget1->GetRenderer()->GetVtkRenderer();
  if (renderer != NULL)
    this->TakeScreenshot(renderer, 1, fileName+"/widget_1.png");

  renderer = m_MultiWidget->mitkWidget2->GetRenderer()->GetVtkRenderer();
  if (renderer != NULL)
    this->TakeScreenshot(renderer, 1, fileName+"/widget_2.png");

  renderer = m_MultiWidget->mitkWidget3->GetRenderer()->GetVtkRenderer();
  if (renderer != NULL)
    this->TakeScreenshot(renderer, 1, fileName+"/widget_3.png");

  n = GetDataStorage()->GetNamedNode("widget1Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 1,0,0 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  n = GetDataStorage()->GetNamedNode("widget2Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 0,1,0 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  n = GetDataStorage()->GetNamedNode("widget3Plane");
  if(n)
  {
    n->SetProperty( "color", mitk::ColorProperty::New( 0,0,1 ) );
//    n->SetProperty("helper object", mitk::BoolProperty::New(false));
  }

  emit EndBlockControlsMovieDeactive();
}

void QmitkScreenshotMaker::Generate3DHighresScreenshot()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");
  GenerateHR3DAtlasScreenshots(fileName);
}

void QmitkScreenshotMaker::GenerateMultiplanar3DHighresScreenshot()
{
  QString fileName = QFileDialog::getExistingDirectory( NULL, "Save screenshots to...", QDir::currentPath());

  GetCam()->Azimuth( -7.5 );
  GetCam()->Roll(-4);
  GenerateHR3DAtlasScreenshots(fileName+"/screen1.png");
  GetCam()->Roll(4);

  GetCam()->Azimuth( 90 );
  GetCam()->Elevation( 4 );
  GenerateHR3DAtlasScreenshots(fileName+"/screen2.png");

  GetCam()->Elevation( 90 );
  GetCam()->Roll( -2.5 );
  GenerateHR3DAtlasScreenshots(fileName+"/screen3.png");

}

void QmitkScreenshotMaker::GenerateHR3DAtlasScreenshots(QString fileName)
{
  emit StartBlockControls();

  // only works correctly for 3D RenderWindow
  vtkRenderer* renderer = m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, this->m_Controls->m_MagFactor->text().toFloat(), fileName);

  emit EndBlockControlsMovieDeactive();
}

vtkCamera* QmitkScreenshotMaker::GetCam()
{
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow());
  vtkCamera* cam = 0;
  const mitk::VtkPropRenderer *propRenderer = dynamic_cast<const mitk::VtkPropRenderer * >( renderer );
  if (propRenderer)
  {
    // get vtk renderer
    vtkRenderer* vtkrenderer = propRenderer->GetVtkRenderer();
    if (vtkrenderer)
    {
      // get vtk camera
      vtkCamera* vtkcam = vtkrenderer->GetActiveCamera();
      if (vtkcam)
      {
        // vtk smart pointer handling
        cam = vtkcam;
        cam->Register( NULL );
      }
    }
  }
  return cam;
}

void QmitkScreenshotMaker::View1()
{
//  GetCam()->Azimuth( -7.5 );
//  GetCam()->Roll(-4);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::View2()
{
//  GetCam()->Azimuth( -7.5 );
//  GetCam()->Roll(-4);
  GetCam()->Azimuth( 90 );
//  GetCam()->Elevation( 4 );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::View3()
{
//  GetCam()->Azimuth( -7.5 );
//  GetCam()->Roll(-4);
  GetCam()->Azimuth( 90 );
//  GetCam()->Elevation( 4 );
  GetCam()->Elevation( 90 );
//  GetCam()->Roll( -2.5 );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if(nodes.size())
    m_SelectedNode = nodes[0];
}

void QmitkScreenshotMaker::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkScreenshotMakerControls;
    m_Controls->setupUi(parent);

    // Initialize "Selected Window" combo box
    const mitk::RenderingManager::RenderWindowVector rwv =
        mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  }

  this->CreateConnections();

}

void QmitkScreenshotMaker::StdMultiWidgetAvailable(QmitkStdMultiWidget&  stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
  m_Parent->setEnabled(true);
}

void QmitkScreenshotMaker::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
  m_Parent->setEnabled(false);
}

void QmitkScreenshotMaker::TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName)
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


//  QColor color = QColorDialog::getColor();
  double bgcolor[] = {m_BackgroundColor.red()/255.0, m_BackgroundColor.green()/255.0, m_BackgroundColor.blue()/255.0};
  renderer->SetBackground(bgcolor);

  m_MultiWidget->DisableColoredRectangles();
  m_MultiWidget->DisableDepartmentLogo();
  m_MultiWidget->DisableGradientBackground();

  m_MultiWidget->mitkWidget1->ActivateMenuWidget( false );
  m_MultiWidget->mitkWidget2->ActivateMenuWidget( false );
  m_MultiWidget->mitkWidget3->ActivateMenuWidget( false );
  m_MultiWidget->mitkWidget4->ActivateMenuWidget( false );

  fileWriter->Write();
  fileWriter->Delete();  

  m_MultiWidget->mitkWidget1->ActivateMenuWidget( true );
  m_MultiWidget->mitkWidget2->ActivateMenuWidget( true );
  m_MultiWidget->mitkWidget3->ActivateMenuWidget( true );
  m_MultiWidget->mitkWidget4->ActivateMenuWidget( true );

  m_MultiWidget->EnableColoredRectangles();
  m_MultiWidget->EnableDepartmentLogo();
  m_MultiWidget->EnableGradientBackground();
  renderer->SetBackground(oldBackground);
  
  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

void QmitkScreenshotMaker::SelectBackgroundColor()
{
  m_BackgroundColor = QColorDialog::getColor();

  m_Controls->m_BackgroundColor->setAutoFillBackground(true);


  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(m_BackgroundColor.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(m_BackgroundColor.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(m_BackgroundColor.blue()));
  styleSheet.append(")");
  m_Controls->m_BackgroundColor->setStyleSheet(styleSheet);
}
