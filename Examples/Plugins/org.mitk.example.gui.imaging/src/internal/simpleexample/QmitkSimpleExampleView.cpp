/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSimpleExampleView.h"
#include <ui_QmitkSimpleExampleViewControls.h>

#include <vtkImageWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkRenderLargeImage.h>

#include "QmitkRenderWindow.h"
#include "QmitkStepperAdapter.h"

#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

const std::string QmitkSimpleExampleView::VIEW_ID = "org.mitk.views.simpleexample";

QmitkSimpleExampleView::QmitkSimpleExampleView()
  : m_Controls(nullptr), m_NavigatorsInitialized(false), m_Parent(nullptr)
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

    this->RenderWindowPartActivated(this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN));
  }
}

void QmitkSimpleExampleView::SetFocus()
{
  m_Controls->renderWindowComboBox->setFocus();
}

void QmitkSimpleExampleView::RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart)
{
  if (renderWindowPart == nullptr)
  {
    m_Parent->setEnabled(false);
    return;
  }

  QHashIterator<QString, QmitkRenderWindow *> renderIter(renderWindowPart->GetQmitkRenderWindows());
  while (renderIter.hasNext())
  {
    renderIter.next();
    m_Controls->renderWindowComboBox->addItem(renderIter.key());
  }

  RenderWindowSelected(m_Controls->renderWindowComboBox->currentText());
  m_TimeStepper.reset(new QmitkStepperAdapter(m_Controls->timeSliceNavigationWidget,
                                              renderWindowPart->GetTimeNavigationController()->GetTime()));
  m_MovieStepper.reset(new QmitkStepperAdapter(m_Controls->movieNavigatorTime,
                                               renderWindowPart->GetTimeNavigationController()->GetTime()));

  m_Parent->setEnabled(true);
}

void QmitkSimpleExampleView::RenderWindowPartDeactivated(mitk::IRenderWindowPart * /*renderWindowPart*/)
{
  m_Parent->setEnabled(false);

  m_SliceStepper.reset();
  m_TimeStepper.reset();
  m_MovieStepper.reset();
  m_Controls->renderWindowComboBox->clear();
}

void QmitkSimpleExampleView::CreateConnections()
{
  if (m_Controls)
  {
    connect(m_Controls->renderWindowComboBox,
            SIGNAL(currentIndexChanged(QString)),
            this,
            SLOT(RenderWindowSelected(QString)));
    connect(m_Controls->stereoSelect, SIGNAL(activated(int)), this, SLOT(StereoSelectionChanged(int)));
    connect(m_Controls->reInitializeNavigatorsButton, SIGNAL(clicked()), this, SLOT(InitNavigators()));
    connect(m_Controls->m_TakeScreenshotBtn, SIGNAL(clicked()), this, SLOT(OnTakeScreenshot()));
    connect(m_Controls->m_TakeHighResScreenShotBtn, SIGNAL(clicked()), this, SLOT(OnTakeHighResolutionScreenshot()));
  }
}

void QmitkSimpleExampleView::InitNavigators()
{
  /* get all nodes that have not set "includeInBoundingBox" to false */
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  /* calculate bounding geometry of these nodes */
  auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);
  /* initialize the views to the bounding geometry */
  m_NavigatorsInitialized = mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

void QmitkSimpleExampleView::StereoSelectionChanged(int id)
{
  /* From vtkRenderWindow.h tells us about stereo rendering:
  Set/Get what type of stereo rendering to use. CrystalEyes mode uses frame-sequential capabilities available in OpenGL
  to drive LCD shutter glasses and stereo projectors. RedBlue mode is a simple type of stereo for use with red-blue
  glasses. Anaglyph mode is a superset of RedBlue mode, but the color output channels can be configured using the
  AnaglyphColorMask and the color of the original image can be (somewhat maintained using AnaglyphColorSaturation; the
  default colors for Anaglyph mode is red-cyan. Interlaced stereo  mode produces a composite image where horizontal
  lines alternate between left and right views. StereoLeft and StereoRight modes choose one or the other stereo view.
  Dresden mode is yet another stereoscopic interleaving.
  */

  auto *renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
  vtkRenderWindow *vtkrenderwindow = renderWindowPart->GetQmitkRenderWindow("3d")->GetVtkRenderWindow();

  // note: foreground vtkRenderers (at least the department logo renderer) produce errors in stereoscopic visualization.
  // Therefore, we disable the logo visualization during stereo rendering.
  switch (id)
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

QmitkRenderWindow *QmitkSimpleExampleView::GetSelectedRenderWindow() const
{
  QString id = m_Controls->renderWindowComboBox->currentText();
  if (id.isEmpty())
  {
    return nullptr;
  }
  else
  {
    return this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow(id);
  }
}

void QmitkSimpleExampleView::OnTakeHighResolutionScreenshot()
{
  QString filter;
  QString fileName = QFileDialog::getSaveFileName(
    nullptr, "Save screenshot to...", QDir::currentPath(), m_PNGExtension + ";;" + m_JPGExtension, &filter);

  vtkRenderer *renderer = this->GetSelectedRenderWindow()->GetRenderer()->GetVtkRenderer();
  if (renderer == nullptr)
    return;
  this->TakeScreenshot(renderer, 4, fileName, filter);
}

void QmitkSimpleExampleView::OnTakeScreenshot()
{
  QString filter;
  QString fileName = QFileDialog::getSaveFileName(
    nullptr, "Save screenshot to...", QDir::currentPath(), m_PNGExtension + ";;" + m_JPGExtension, &filter);

  QmitkRenderWindow *renWin = this->GetSelectedRenderWindow();
  if (renWin == nullptr)
    return;

  vtkRenderer *renderer = renWin->GetRenderer()->GetVtkRenderer();
  if (renderer == nullptr)
    return;
  this->TakeScreenshot(renderer, 1, fileName, filter);
}

void QmitkSimpleExampleView::TakeScreenshot(vtkRenderer *renderer,
                                            unsigned int magnificationFactor,
                                            QString fileName,
                                            QString filter)
{
  if ((renderer == nullptr) || (magnificationFactor < 1) || fileName.isEmpty())
    return;

  bool doubleBuffering(renderer->GetRenderWindow()->GetDoubleBuffer());
  renderer->GetRenderWindow()->DoubleBufferOff();

  vtkImageWriter *fileWriter = nullptr;

  QFileInfo fi(fileName);
  QString suffix = fi.suffix().toLower();

  if (suffix.isEmpty() || (suffix != "png" && suffix != "jpg" && suffix != "jpeg"))
  {
    if (filter == m_PNGExtension)
    {
      suffix = "png";
    }
    else if (filter == m_JPGExtension)
    {
      suffix = "jpg";
    }
    fileName += "." + suffix;
  }

  if (suffix.compare("jpg", Qt::CaseInsensitive) == 0 || suffix.compare("jpeg", Qt::CaseInsensitive) == 0)
  {
    vtkJPEGWriter *w = vtkJPEGWriter::New();
    w->SetQuality(100);
    w->ProgressiveOff();
    fileWriter = w;
  }
  else // default is png
  {
    fileWriter = vtkPNGWriter::New();
  }

  vtkRenderLargeImage *magnifier = vtkRenderLargeImage::New();
  magnifier->SetInput(renderer);
  magnifier->SetMagnification(magnificationFactor);
  fileWriter->SetInputConnection(magnifier->GetOutputPort());
  fileWriter->SetFileName(fileName.toLatin1());

  // vtkRenderLargeImage has problems with different layers, therefore we have to
  // temporarily deactivate all other layers.
  // we set the background to white, because it is nicer than black...
  double oldBackground[3];
  renderer->GetBackground(oldBackground);
  double white[] = {1.0, 1.0, 1.0};
  renderer->SetBackground(white);
  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN);
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
    m_SliceStepper.reset(new QmitkStepperAdapter(m_Controls->sliceNavigationWidget,
      this->GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow(id)->GetSliceNavigationController()->GetSlice()));
  }
}
