/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkSimpleExampleView.h"

#include "QmitkStepperAdapter.h"
#include "QmitkRenderWindow.h"

#include "mitkMovieGenerator.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateNot.h"
#include "mitkProperties.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>


#include <vtkRenderWindow.h>
#include <vtkImageWriter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkRenderLargeImage.h>

const std::string QmitkSimpleExampleView::VIEW_ID = "org.mitk.views.simpleexample";

QmitkSimpleExampleView::QmitkSimpleExampleView()
: m_Controls(NULL),
  m_NavigatorsInitialized(false),
  m_Parent(NULL)
{
}

QmitkSimpleExampleView::~QmitkSimpleExampleView()
{
}

void QmitkSimpleExampleView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Parent = parent;
    // create GUI widgets
    m_Controls = new Ui::QmitkSimpleExampleViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    this->RenderWindowPartActivated(this->GetRenderWindowPart());
  }
}

void QmitkSimpleExampleView::SetFocus()
{
  m_Controls->renderWindowComboBox->setFocus();
}

void QmitkSimpleExampleView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (renderWindowPart == NULL)
  {
    m_Parent->setEnabled(false);
    return;
  }

  QHashIterator<QString, QmitkRenderWindow*> renderIter(renderWindowPart->GetQmitkRenderWindows());
  while(renderIter.hasNext())
  {
    renderIter.next();
    m_Controls->renderWindowComboBox->addItem(renderIter.key());
  }

  RenderWindowSelected(m_Controls->renderWindowComboBox->currentText());
  m_TimeStepper.reset(new QmitkStepperAdapter(m_Controls->sliceNavigatorTime, renderWindowPart->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromSimpleExample"));
  m_MovieStepper.reset(new QmitkStepperAdapter(m_Controls->movieNavigatorTime, renderWindowPart->GetTimeNavigationController()->GetTime(), "movieNavigatorTimeFromSimpleExample"));

  m_Parent->setEnabled(true);
}

void QmitkSimpleExampleView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_Parent->setEnabled(false);

  m_SliceStepper.reset();
  m_TimeStepper.reset();
  m_MovieStepper.reset();
  m_Controls->renderWindowComboBox->clear();
}

void QmitkSimpleExampleView::CreateConnections()
{
  if ( m_Controls )
  {
    connect(m_Controls->renderWindowComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(RenderWindowSelected(QString)) );
    connect(m_Controls->stereoSelect, SIGNAL(activated(int)), this, SLOT(StereoSelectionChanged(int)) );
    connect(m_Controls->reInitializeNavigatorsButton, SIGNAL(clicked()), this, SLOT(InitNavigators()) );
    connect(m_Controls->genMovieButton, SIGNAL(clicked()), this, SLOT(GenerateMovie()) );
    connect(m_Controls->m_TakeScreenshotBtn, SIGNAL(clicked()), this, SLOT(OnTakeScreenshot()) );
    connect(m_Controls->m_TakeHighResScreenShotBtn, SIGNAL(clicked()), this, SLOT(OnTakeHighResolutionScreenshot()) );
  }
}

void QmitkSimpleExampleView::InitNavigators()
{
  /* get all nodes that have not set "includeInBoundingBox" to false */
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  /* calculate bounding geometry of these nodes */
  mitk::TimeGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);
  /* initialize the views to the bounding geometry */
  m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  //m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()->InitializeViews(GetDefaultDataStorage());
}

void QmitkSimpleExampleView::GenerateMovie()
{
  QmitkRenderWindow* movieRenderWindow = GetSelectedRenderWindow();
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

void QmitkSimpleExampleView::StereoSelectionChanged( int id )
{
  /* From vtkRenderWindow.h tells us about stereo rendering:
  Set/Get what type of stereo rendering to use. CrystalEyes mode uses frame-sequential capabilities available in OpenGL to drive LCD shutter glasses and stereo projectors. RedBlue mode is a simple type of stereo for use with red-blue glasses. Anaglyph mode is a superset of RedBlue mode, but the color output channels can be configured using the AnaglyphColorMask and the color of the original image can be (somewhat maintained using AnaglyphColorSaturation; the default colors for Anaglyph mode is red-cyan. Interlaced stereo  mode produces a composite image where horizontal lines alternate between left and right views. StereoLeft and StereoRight modes choose one or the other stereo view. Dresden mode is yet another stereoscopic interleaving.
  */

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  vtkRenderWindow * vtkrenderwindow = this->GetSelectedRenderWindow()->GetVtkRenderWindow();

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
    renderWindowPart->EnableDecorations(false, QStringList(mitk::IRenderWindowPart::DECORATION_LOGO));
    break;
  case 2:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    renderWindowPart->EnableDecorations(false, QStringList(mitk::IRenderWindowPart::DECORATION_LOGO));
    break;
  }


  mitk::BaseRenderer::GetInstance(vtkrenderwindow)->SetMapperID(mitk::BaseRenderer::Standard3D);
  renderWindowPart->RequestUpdate();
}

QmitkRenderWindow* QmitkSimpleExampleView::GetSelectedRenderWindow() const
{
  QString id = m_Controls->renderWindowComboBox->currentText();
  if (id.isEmpty())
  {
    return NULL;
  }
  else
  {
    return this->GetRenderWindowPart()->GetQmitkRenderWindow(id);
  }
}

void QmitkSimpleExampleView::OnTakeHighResolutionScreenshot()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");

  vtkRenderer* renderer = this->GetSelectedRenderWindow()->GetRenderer()->GetVtkRenderer();
  if (renderer == NULL)
    return;
  this->TakeScreenshot(renderer, 4, fileName);
}

void QmitkSimpleExampleView::OnTakeScreenshot()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", QDir::currentPath(), "JPEG file (*.jpg);;PNG file (*.png)");

  QmitkRenderWindow* renWin = this->GetSelectedRenderWindow();
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
  fileWriter->SetInputConnection(magnifier->GetOutputPort());
  fileWriter->SetFileName(fileName.toLatin1());

  // vtkRenderLargeImage has problems with different layers, therefore we have to
  // temporarily deactivate all other layers.
  // we set the background to white, because it is nicer than black...
  double oldBackground[3];
  renderer->GetBackground(oldBackground);
  double white[] = {1.0, 1.0, 1.0};
  renderer->SetBackground(white);
  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  renderWindowPart->EnableDecorations(false);

  fileWriter->Write();
  fileWriter->Delete();

  renderWindowPart->EnableDecorations(true);

  renderer->SetBackground(oldBackground);

  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

void QmitkSimpleExampleView::RenderWindowSelected(const QString &id)
{
  if (!id.isEmpty())
  {
    m_SliceStepper.reset(new QmitkStepperAdapter(m_Controls->sliceNavigator,
                                                 this->GetRenderWindowPart()->GetQmitkRenderWindow(id)->GetSliceNavigationController()->GetSlice(),
                                                 "sliceNavigatorFromSimpleExample"));
  }
}
