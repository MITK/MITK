/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOpenCVVideoControls.h"
#include <QmitkVideoBackground.h>
#include <QmitkRenderWindow.h>
#include <mitkOpenCVVideoSource.h>

#include <mitkIPersistenceService.h>

class QmitkOpenCVVideoControlsPrivate
{
public:

  QmitkOpenCVVideoControlsPrivate(QmitkOpenCVVideoControls* q, const std::string& id)
    : q(q)
    , m_Id(id)
  {}

  ///
  /// muellerm: persitence service implementation
  ///
  PERSISTENCE_GET_SERVICE_METHOD_MACRO

    QmitkOpenCVVideoControls* q;

  ///
  /// muellerm: a unique id for the prop list
  ///
  std::string m_Id;
  void ToPropertyList();
  void FromPropertyList();
};

QmitkOpenCVVideoControls::QmitkOpenCVVideoControls(QmitkVideoBackground* _VideoBackground
  , QmitkRenderWindow* _RenderWindow
  , QWidget * parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_VideoBackground(nullptr)
  , m_RenderWindow(nullptr)
  , m_VideoSource(nullptr)
  , m_Controls(new Ui::QmitkOpenCVVideoControls)
  , m_SliderCurrentlyMoved(false)
  , d(new QmitkOpenCVVideoControlsPrivate(this, "QmitkOpenCVVideoControls"))
{
  m_Controls->setupUi(this);
  m_Controls->FileChooser->SetFileMustExist(true);
  m_Controls->FileChooser->SetSelectDir(false);

  this->SetRenderWindow(_RenderWindow);
  this->SetVideoBackground(_VideoBackground);
  d->FromPropertyList();

  mitk::IPersistenceService* persistenceService = d->GetPersistenceService();

  if (persistenceService != nullptr)
  {
    persistenceService->AddPropertyListReplacedObserver(this);
  }
  else
  {
    MITK_WARN << "No Persistence Service available in constructor";
  }
}

QmitkOpenCVVideoControls::~QmitkOpenCVVideoControls()
{
  if (m_VideoSource != nullptr && m_VideoSource->IsCapturingEnabled())
  {
    this->Stop(); // emulate stop
  }

  mitk::IPersistenceService* persistenceService = d->GetPersistenceService();
  if (persistenceService != nullptr)
  {
    persistenceService->RemovePropertyListReplacedObserver(this);
  }
  else
  {
    MITK_WARN << "No Persistence Service available in destructor";
  }

  d->ToPropertyList();
}

void QmitkOpenCVVideoControls::on_VideoProgressSlider_valueChanged(int /*value*/)
{
  //Fixes T23169
  on_VideoProgressSlider_sliderReleased();
}

void QmitkOpenCVVideoControls::on_UseGrabbingDeviceButton_clicked(bool /*checked=false*/)
{
  m_Controls->GrabbingDevicePanel->setEnabled(true);
  m_Controls->VideoFilePanel->setEnabled(false);
}

void QmitkOpenCVVideoControls::on_UseVideoFileButton_clicked(bool /*checked=false*/)
{
  m_Controls->GrabbingDevicePanel->setEnabled(false);
  m_Controls->VideoFilePanel->setEnabled(true);
  m_Controls->FileChooser->setEnabled(true);
}

void QmitkOpenCVVideoControls::on_VideoProgressSlider_sliderPressed()
{
  m_SliderCurrentlyMoved = true;
  // temporary pause the video while sliding
  if (!m_VideoSource->GetCapturePaused())
    m_VideoSource->PauseCapturing();
}

void QmitkOpenCVVideoControls::on_VideoProgressSlider_sliderReleased()
{
  double progressRatio = static_cast<double>(m_Controls->VideoProgressSlider->value())
    / static_cast<double>(m_Controls->VideoProgressSlider->maximum());
  m_VideoSource->SetVideoCaptureProperty(CV_CAP_PROP_POS_FRAMES, progressRatio*m_VideoSource->GetVideoCaptureProperty(CV_CAP_PROP_FRAME_COUNT));

  // resume the video ( if it was not paused by the user)
  if (m_VideoSource->GetCapturePaused() && m_Controls->PlayButton->isChecked())
    m_VideoSource->PauseCapturing();
  m_SliderCurrentlyMoved = false;
}

void QmitkOpenCVVideoControls::on_RepeatVideoButton_clicked(bool checked)
{
  MITK_INFO << "repeat video clicked";
  m_VideoSource->SetRepeatVideo(checked);
}

void QmitkOpenCVVideoControls::on_PlayButton_clicked(bool checked)
{
  MITK_INFO << "play button clicked";
  if (checked)
  {
    this->Play();
  }
  else
  {
    // show pause button
    this->IsPlaying(true);
    m_VideoSource->PauseCapturing();
  }
}

void QmitkOpenCVVideoControls::on_StopButton_clicked(bool /*checked=false*/)
{
  this->Stop();
}

void QmitkOpenCVVideoControls::Play()
{
  if (m_VideoSource->GetCapturePaused())
  {
    this->IsPlaying(false);
    m_VideoSource->PauseCapturing();
  }
  else
  {
    if (m_Controls->UseGrabbingDeviceButton->isChecked())
    {
      m_VideoSource->SetVideoCameraInput(m_Controls->GrabbingDeviceNumber->text().toInt(), false);
      m_Controls->VideoFileControls->setEnabled(false);
    }
    else
    {
      m_VideoSource->SetVideoFileInput(m_Controls->FileChooser->GetFile().c_str(), m_Controls->RepeatVideoButton->isChecked(), false);
      m_VideoSource->SetRepeatVideo(m_Controls->RepeatVideoButton->isChecked());
      m_Controls->VideoProgressSlider->setValue(0);
    }

    m_VideoSource->StartCapturing();

    if (!m_VideoSource->IsCapturingEnabled())
    {
      MITK_ERROR << "Video could not be initialized!";
      m_Controls->PlayButton->setChecked(false);
    }
    else
    {
      int hertz = m_Controls->UpdateRate->text().toInt();
      int updateTime = itk::Math::Round<int, double>(1000.0 / hertz);

      // resets the whole background
      m_VideoBackground->SetTimerDelay(updateTime);
      m_VideoBackground->AddRenderWindow(m_RenderWindow->GetVtkRenderWindow());
      this->connect(m_VideoBackground, SIGNAL(NewFrameAvailable(mitk::VideoSource*))
        , this, SLOT(NewFrameAvailable(mitk::VideoSource*)));
      this->connect(m_VideoBackground, SIGNAL(EndOfVideoSourceReached(mitk::VideoSource*))
        , this, SLOT(EndOfVideoSourceReached(mitk::VideoSource*)));

      m_VideoBackground->Enable();
      this->m_Controls->StopButton->setEnabled(true);
      // show video file controls
      if (m_Controls->UseVideoFileButton->isChecked())
      {
        m_Controls->VideoFileControls->setEnabled(true);
        m_Controls->RepeatVideoButton->setEnabled(true);
        m_Controls->VideoProgressSlider->setEnabled(true);
      }
      // show pause button
      this->IsPlaying(false);
      // disable other controls
      m_Controls->GrabbingDevicePanel->setEnabled(false);
      m_Controls->VideoFilePanel->setEnabled(false);
      m_Controls->UseGrabbingDeviceButton->setEnabled(false);
      m_Controls->UseVideoFileButton->setEnabled(false);
      m_Controls->UpdateRatePanel->setEnabled(false);
    }
  }
}

void QmitkOpenCVVideoControls::Stop()
{
  // disable video file controls, stop button and show play button again
  m_Controls->UseGrabbingDeviceButton->setEnabled(true);
  m_Controls->UseVideoFileButton->setEnabled(true);
  if (m_Controls->UseGrabbingDeviceButton->isChecked())
    on_UseGrabbingDeviceButton_clicked(true);
  else
    on_UseVideoFileButton_clicked(true);

  m_Controls->UpdateRatePanel->setEnabled(true);
  m_Controls->VideoProgressSlider->setValue(0);
  m_Controls->VideoFileControls->setEnabled(false);
  this->m_Controls->StopButton->setEnabled(false);
  this->IsPlaying(true);

  if (m_VideoBackground)
  {
    m_VideoBackground->Disable();

    if (m_RenderWindow)
      m_VideoBackground->RemoveRenderWindow(m_RenderWindow->GetVtkRenderWindow());

    this->disconnect(m_VideoBackground, SIGNAL(NewFrameAvailable(mitk::VideoSource*))
      , this, SLOT(NewFrameAvailable(mitk::VideoSource*)));
  }
  if (m_VideoSource != nullptr)
    m_VideoSource->StopCapturing();
}

void QmitkOpenCVVideoControls::Reset()
{
  this->Stop();
}

void QmitkOpenCVVideoControls::IsPlaying(bool paused)
{
  if (paused)
  {
    m_Controls->PlayButton->setText("Play");
    m_Controls->PlayButton->setIcon(QIcon(":/OpenCVVideoSupportUI/media-playback-start.png"));
    m_Controls->PlayButton->setChecked(false);
  }
  else
  {
    m_Controls->PlayButton->setText("Pause");
    m_Controls->PlayButton->setIcon(QIcon(":/OpenCVVideoSupportUI/media-playback-pause.png"));
    m_Controls->PlayButton->setChecked(true);
  }
}

void QmitkOpenCVVideoControls::NewFrameAvailable(mitk::VideoSource* /*videoSource*/)
{
  emit NewOpenCVFrameAvailable(m_VideoSource->GetCurrentFrame());
  if (!m_SliderCurrentlyMoved)
  {
    m_Controls->VideoProgressSlider->setValue(itk::Math::Round<int, double>(m_VideoSource->GetVideoCaptureProperty(CV_CAP_PROP_POS_FRAMES)
      *(1 / m_VideoSource->GetVideoCaptureProperty(CV_CAP_PROP_FRAME_COUNT)
        *m_Controls->VideoProgressSlider->maximum())));
  }
}

void QmitkOpenCVVideoControls::EndOfVideoSourceReached(mitk::VideoSource* /*videoSource*/)
{
  if (m_Controls->RepeatVideoButton->isChecked())
  {
    this->Reset();
    this->Play();
  }
  else
  {
    this->Stop();
  }
}

void QmitkOpenCVVideoControls::SetRenderWindow(QmitkRenderWindow* _RenderWindow)
{
  if (m_RenderWindow == _RenderWindow)
    return;

  // In Reset() m_MultiWidget is used, set it to 0 now for avoiding errors
  if (_RenderWindow == nullptr)
    m_RenderWindow = nullptr;
  this->Reset();

  m_RenderWindow = _RenderWindow;

  if (m_RenderWindow == nullptr)
  {
    this->setEnabled(false);
  }
  else
  {
    this->setEnabled(true);
  }
}

QmitkRenderWindow* QmitkOpenCVVideoControls::GetRenderWindow() const
{
  return m_RenderWindow;
}

void QmitkOpenCVVideoControls::SetVideoBackground(QmitkVideoBackground* _VideoBackground)
{
  if (m_VideoBackground == _VideoBackground)
    return;

  if (m_VideoBackground != nullptr)
    this->disconnect(m_VideoBackground, SIGNAL(destroyed(QObject*))
      , this, SLOT(QObjectDestroyed(QObject*)));

  this->Reset();

  m_VideoBackground = _VideoBackground;

  if (m_VideoBackground == nullptr)
  {
    m_VideoSource = nullptr;
    MITK_WARN << "m_MultiWidget is 0";
    this->setEnabled(false);
  }
  else
  {
    this->setEnabled(true);
    m_VideoSource = dynamic_cast<mitk::OpenCVVideoSource*>(m_VideoBackground->GetVideoSource());
    // preset form entries
    if (m_VideoSource != nullptr)
    {
      if (!m_VideoSource->GetVideoFileName().empty())
      {
        m_Controls->FileChooser->SetFile(m_VideoSource->GetVideoFileName());
        on_UseGrabbingDeviceButton_clicked(false);
      }
      else if (m_VideoSource->GetGrabbingDeviceNumber() >= 0)
        m_Controls->GrabbingDeviceNumber->setValue(m_VideoSource->GetGrabbingDeviceNumber());

      m_Controls->UpdateRate->setValue(m_VideoBackground->GetTimerDelay());

      this->connect(m_VideoBackground, SIGNAL(destroyed(QObject*))
        , this, SLOT(QObjectDestroyed(QObject*)));
    }
    else
    {
      MITK_WARN << "m_VideoSource is 0";
      this->setEnabled(false);
    }
  }
}

QmitkVideoBackground* QmitkOpenCVVideoControls::GetVideoBackground() const
{
  return m_VideoBackground;
}

void QmitkOpenCVVideoControls::QObjectDestroyed(QObject * obj /*= 0 */)
{
  if (m_VideoBackground == obj)
  {
    m_VideoSource = nullptr;
    this->SetVideoBackground(nullptr);
  }
}

void QmitkOpenCVVideoControlsPrivate::ToPropertyList()
{
  mitk::IPersistenceService* persistenceService = this->GetPersistenceService();

  if (persistenceService != nullptr)
  {
    mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(m_Id);
    propList->Set("deviceType", q->m_Controls->UseGrabbingDeviceButton->isChecked() ? 0 : 1);
    propList->Set("grabbingDeviceNumber", q->m_Controls->GrabbingDeviceNumber->value());
    propList->Set("updateRate", q->m_Controls->UpdateRate->value());
    propList->Set("repeatVideo", q->m_Controls->RepeatVideoButton->isChecked());
  }
  else
  {
    MITK_WARN << "Persistence Service not available.";
  }
}

void QmitkOpenCVVideoControlsPrivate::FromPropertyList()
{
  mitk::IPersistenceService* persistenceService = this->GetPersistenceService();

  if (persistenceService != nullptr)
  {
    mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(m_Id);

    bool repeatVideo = false;
    propList->Get("repeatVideo", repeatVideo);
    q->m_Controls->RepeatVideoButton->setChecked(repeatVideo);

    int updateRate = 25;
    propList->Get("updateRate", updateRate);
    q->m_Controls->UpdateRate->setValue(updateRate);

    int grabbingDeviceNumber = 0;
    propList->Get("grabbingDeviceNumber", grabbingDeviceNumber);
    q->m_Controls->GrabbingDeviceNumber->setValue(grabbingDeviceNumber);

    int deviceType = 0;
    propList->Get("deviceType", deviceType);
    if (deviceType == 0)
    {
      q->m_Controls->UseGrabbingDeviceButton->setChecked(true);
    }
    else
    {
      q->m_Controls->UseVideoFileButton->setChecked(true);
    }
  }
  else
  {
    MITK_WARN << "Persistence Service not available.";
  }
}

void QmitkOpenCVVideoControls::AfterPropertyListReplaced(const std::string& id, mitk::PropertyList* /*propertyList*/)
{
  if (id == d->m_Id)
    d->FromPropertyList();
}
