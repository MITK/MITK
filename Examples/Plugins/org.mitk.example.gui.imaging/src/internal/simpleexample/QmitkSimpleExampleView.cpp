/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSimpleExampleView.h"

#include <vtkImageWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGL.h>

#include "QmitkRenderWindow.h"
#include "QmitkStepperAdapter.h"

#include "QmitkFFmpegWriter.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <berryPlatform.h>

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

    this->RenderWindowPartActivated(this->GetRenderWindowPart());
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
  m_TimeStepper.reset(new QmitkStepperAdapter(m_Controls->sliceNavigatorTime,
                                              renderWindowPart->GetTimeNavigationController()->GetTime(),
                                              "sliceNavigatorTimeFromSimpleExample"));
  m_MovieStepper.reset(new QmitkStepperAdapter(m_Controls->movieNavigatorTime,
                                               renderWindowPart->GetTimeNavigationController()->GetTime(),
                                               "movieNavigatorTimeFromSimpleExample"));

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
    connect(m_Controls->genMovieButton, SIGNAL(clicked()), this, SLOT(GenerateMovie()));
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

/**
 * Returns path to the ffmpeg lib if configured in preferences.
 *
 * This implementation has been reused from MovieMaker view.
 *
 * @return The path to ffmpeg lib or empty string if not configured.
 */
QString QmitkSimpleExampleView::GetFFmpegPath() const
{
  berry::IPreferences::Pointer preferences =
    berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.gui.qt.ext.externalprograms");

  return preferences.IsNotNull() ? preferences->Get("ffmpeg", "") : "";
}

/**
 * Reads pixels from specified render window.
 *
 * This implementation has been reused from MovieMaker view.
 *
 * @param renderWindow
 * @param x
 * @param y
 * @param width
 * @param height
 * @return
 */
static unsigned char *ReadPixels(vtkRenderWindow *renderWindow, int x, int y, int width, int height)
{
  if (renderWindow == nullptr)
    return nullptr;

  unsigned char *frame = new unsigned char[width * height * 3];

  renderWindow->MakeCurrent();
  glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame);

  return frame;
}

/**
 * Records a movie from the selected render window with a default frame rate of 30 Hz.
 *
 * Parts of this implementation have been reused from MovieMaker view.
 */
void QmitkSimpleExampleView::GenerateMovie()
{
  QmitkRenderWindow *movieRenderWindow = GetSelectedRenderWindow();

  mitk::Stepper::Pointer stepper = movieRenderWindow->GetSliceNavigationController()->GetSlice();

  QmitkFFmpegWriter *movieWriter = new QmitkFFmpegWriter(m_Parent);

  const QString ffmpegPath = GetFFmpegPath();

  if (ffmpegPath.isEmpty())
  {
    QMessageBox::information(
      nullptr,
      "Movie Maker",
      "<p>Set path to FFmpeg<sup>1</sup> in preferences (Window -> Preferences... "
      "(Ctrl+P) -> External Programs) to be able to record your movies to video files.</p>"
      "<p>If you are using Linux, chances are good that FFmpeg is included in the official package "
      "repositories.</p>"
      "<p>[1] <a href=\"https://www.ffmpeg.org/download.html\">Download FFmpeg from ffmpeg.org</a></p>");
    return;
  }

  movieWriter->SetFFmpegPath(GetFFmpegPath());

  vtkRenderWindow *renderWindow = movieRenderWindow->renderWindow();

  if (renderWindow == nullptr)
    return;

  const int border = 3;
  const int x = border;
  const int y = border;
  int width = renderWindow->GetSize()[0] - border * 2;
  int height = renderWindow->GetSize()[1] - border * 2;

  if (width & 1)
    --width;

  if (height & 1)
    --height;

  if (width < 16 || height < 16)
    return;

  movieWriter->SetSize(width, height);
  movieWriter->SetFramerate(30);

  QString saveFileName = QFileDialog::getSaveFileName(nullptr, "Specify a filename", "", "Movie (*.mp4)");

  if (saveFileName.isEmpty())
    return;

  if (!saveFileName.endsWith(".mp4"))
    saveFileName += ".mp4";

  movieWriter->SetOutputPath(saveFileName);

  const unsigned int numberOfFrames = stepper->GetSteps() - stepper->GetPos();

  try
  {
    movieWriter->Start();

    for (unsigned int currentFrame = 0; currentFrame < numberOfFrames; ++currentFrame)
    {
      mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

      renderWindow->MakeCurrent();
      unsigned char *frame = ReadPixels(renderWindow, x, y, width, height);
      movieWriter->WriteFrame(frame);
      delete[] frame;

      stepper->Next();
    }

    movieWriter->Stop();

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
  catch (const mitk::Exception &exception)
  {
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

    QMessageBox::critical(nullptr, "Generate Movie", exception.GetDescription());
  }
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

  mitk::IRenderWindowPart *renderWindowPart = this->GetRenderWindowPart();
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
    return this->GetRenderWindowPart()->GetQmitkRenderWindow(id);
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
  mitk::IRenderWindowPart *renderWindowPart = this->GetRenderWindowPart();
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
    m_SliceStepper.reset(new QmitkStepperAdapter(
      m_Controls->sliceNavigator,
      this->GetRenderWindowPart()->GetQmitkRenderWindow(id)->GetSliceNavigationController()->GetSlice(),
      "sliceNavigatorFromSimpleExample"));
  }
}
