/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkOpenCVVideoControls.h"
#include <QmitkVideoBackground.h>
#include <QmitkStdMultiWidget.h>
#include <mitkOpenCVVideoSource.h>

QmitkOpenCVVideoControls::QmitkOpenCVVideoControls( QmitkVideoBackground* _VideoBackground
                                                   , QmitkStdMultiWidget* _MultiWidget
                                                   , QWidget * parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_VideoBackground(0)
, m_MultiWidget(0)
, m_VideoSource(0)
, m_Controls(new Ui::QmitkOpenCVVideoControls)
, m_SliderCurrentlyMoved(false)
{
  m_Controls->setupUi(this);
  m_Controls->FileChooser->SetFileMustExist(true);
  m_Controls->FileChooser->SetSelectDir(false);

  this->SetStdMultiWidget(_MultiWidget);
  this->SetVideoBackground(_VideoBackground);
}

QmitkOpenCVVideoControls::~QmitkOpenCVVideoControls()
{
  if(m_VideoSource != 0 && m_VideoSource->IsCapturingEnabled())
    this->Stop(); // emulate stop
}

void QmitkOpenCVVideoControls::on_UseGrabbingDeviceButton_clicked( bool checked/*=false*/ )
{
  m_Controls->GrabbingDevicePanel->setEnabled(true);
  m_Controls->VideoFilePanel->setEnabled(false);
}

void QmitkOpenCVVideoControls::on_UseVideoFileButton_clicked( bool checked/*=false*/ )
{
  m_Controls->GrabbingDevicePanel->setEnabled(false);
  m_Controls->VideoFilePanel->setEnabled(true);
  m_Controls->FileChooser->setEnabled(true);
}

//void QmitkOpenCVVideoControls::on_VideoProgressSlider_sliderMoved( int value )
//{
//  MITK_DEBUG << "progress bar slider clicked";
//  double progressRatio = static_cast<double>(value)/static_cast<double>(m_Controls->VideoProgressSlider->maximum());
//  MITK_DEBUG << "progressRatio" << progressRatio;
//  m_VideoSource->SetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO, progressRatio);
//}

void QmitkOpenCVVideoControls::on_VideoProgressSlider_sliderPressed()
{
  m_SliderCurrentlyMoved = true;
  // temporary pause the video while sliding
  if( !m_VideoSource->GetCapturePaused() )
    m_VideoSource->PauseCapturing();
  MITK_DEBUG << "freezing video with old pos ratio: "<< m_VideoSource->GetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO);
}

void QmitkOpenCVVideoControls::on_VideoProgressSlider_sliderReleased()
{
  double progressRatio = static_cast<double>(m_Controls->VideoProgressSlider->value())
    /static_cast<double>(m_Controls->VideoProgressSlider->maximum());
  m_VideoSource->SetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO, progressRatio);

  MITK_DEBUG << "resuming video with new pos ratio: "<< progressRatio;
  // resume the video ( if it was not paused by the user)
  if( m_VideoSource->GetCapturePaused() && m_Controls->PlayButton->isChecked() )
    m_VideoSource->PauseCapturing();
  m_SliderCurrentlyMoved = false;
}

void QmitkOpenCVVideoControls::on_RepeatVideoButton_clicked( bool checked/*=false */ )
{
  MITK_INFO << "repeat video clicked";
  m_VideoSource->SetRepeatVideo(checked);
}

void QmitkOpenCVVideoControls::on_PlayButton_clicked( bool checked/*=false*/ )
{
  MITK_INFO << "play button clicked";
  if(checked)
  {
    if( m_VideoSource->GetCapturePaused() )
    {
      this->SwitchPlayButton(false);
      m_VideoSource->PauseCapturing();
    }
    else
    {
      if(m_Controls->UseGrabbingDeviceButton->isChecked())
      {
        m_VideoSource->SetVideoCameraInput(m_Controls->GrabbingDeviceNumber->text().toInt(),false);
        m_Controls->VideoFileControls->setEnabled(false);
      }
      else
      {
        m_VideoSource->SetVideoFileInput(m_Controls->FileChooser->GetFile().c_str(), m_Controls->RepeatVideoButton->isChecked(), false);
        m_VideoSource->SetRepeatVideo(m_Controls->RepeatVideoButton->isChecked());
        m_Controls->VideoProgressSlider->setValue(0);
      }

      m_VideoSource->StartCapturing();
      if(!m_VideoSource->IsCapturingEnabled())
      {
        MITK_ERROR << "Video could not be initialized!";
        m_Controls->PlayButton->setChecked(false);
      }

      else
      {
        int hertz = m_Controls->UpdateRate->text().toInt();
        int updateTime = itk::Math::Round( static_cast<double>(1000.0/hertz) );

        // resets the whole background
        m_VideoBackground->SetTimerDelay( updateTime );
        m_VideoBackground->AddRenderWindow( m_MultiWidget->GetRenderWindow4()->GetRenderWindow() );
        this->connect( m_VideoBackground, SIGNAL(NewFrameAvailable(mitk::VideoSource*))
          , this, SLOT(NewFrameAvailable(mitk::VideoSource*)));
        m_MultiWidget->DisableGradientBackground();

        m_VideoBackground->Enable();
        this->m_Controls->StopButton->setEnabled(true);
        // show video file controls
        if(m_Controls->UseVideoFileButton->isChecked())
        {
          m_Controls->VideoFileControls->setEnabled(true);
          m_Controls->RepeatVideoButton->setEnabled(true);
          m_Controls->VideoProgressSlider->setEnabled(true);
        }
        // show pause button
        this->SwitchPlayButton(false);
        // disable other controls
        m_Controls->GrabbingDevicePanel->setEnabled(false);
        m_Controls->VideoFilePanel->setEnabled(false);
        m_Controls->UseGrabbingDeviceButton->setEnabled(false);
        m_Controls->UseVideoFileButton->setEnabled(false);
        m_Controls->UpdateRatePanel->setEnabled(false);
      }
    }
  }
  else
  {
    // show pause button
    this->SwitchPlayButton(true);
    m_VideoSource->PauseCapturing();

  }
}

void QmitkOpenCVVideoControls::on_StopButton_clicked( bool checked/*=false*/ )
{
  this->Stop();
}

void QmitkOpenCVVideoControls::Stop()
{
  // disable video file controls, stop button and show play button again
  m_Controls->UseGrabbingDeviceButton->setEnabled(true);
  m_Controls->UseVideoFileButton->setEnabled(true);
  if(m_Controls->UseGrabbingDeviceButton->isChecked())
    on_UseGrabbingDeviceButton_clicked(true);
  else
    on_UseVideoFileButton_clicked(true);

  m_Controls->UpdateRatePanel->setEnabled(true);
  m_Controls->VideoFileControls->setEnabled(false);
  this->m_Controls->StopButton->setEnabled(false);
  this->SwitchPlayButton(true);

  if(m_MultiWidget)
    m_MultiWidget->EnableGradientBackground();
  if(m_VideoBackground)
  {
    m_VideoBackground->Disable();

    if(m_MultiWidget)
      m_VideoBackground->RemoveRenderWindow( m_MultiWidget->GetRenderWindow4()->GetRenderWindow() );

    this->disconnect( m_VideoBackground, SIGNAL(NewFrameAvailable(mitk::VideoSource*))
      , this, SLOT(NewFrameAvailable(mitk::VideoSource*)));
  }
  if(m_VideoSource != 0)
    m_VideoSource->StopCapturing();
}

void QmitkOpenCVVideoControls::Reset()
{
  this->Stop();
}

void QmitkOpenCVVideoControls::SwitchPlayButton(bool paused)
{
  if(paused)
  {
    m_Controls->PlayButton->setText("Play");
    m_Controls->PlayButton->setIcon( QIcon(":/OpenCVVideoSupportUI/media-playback-start.png") );
    m_Controls->PlayButton->setChecked(false);
  }
  else
  {
    m_Controls->PlayButton->setText("Pause");
    m_Controls->PlayButton->setIcon( QIcon(":/OpenCVVideoSupportUI/media-playback-pause.png") );
    m_Controls->PlayButton->setChecked(true);
  }
}

void QmitkOpenCVVideoControls::NewFrameAvailable( mitk::VideoSource* videoSource )
{
  emit NewOpenCVFrameAvailable( m_VideoSource->GetCurrentFrame() );
  if(!m_SliderCurrentlyMoved)
    m_Controls->VideoProgressSlider->setValue( itk::Math::Round( m_VideoSource->GetVideoCaptureProperty(CV_CAP_PROP_POS_AVI_RATIO)
      *m_Controls->VideoProgressSlider->maximum() ) );
}

void QmitkOpenCVVideoControls::SetStdMultiWidget( QmitkStdMultiWidget* _MultiWidget )
{
  if(m_MultiWidget == _MultiWidget)
    return;

  if(m_MultiWidget != 0)
    this->disconnect( m_MultiWidget, SIGNAL(destroyed(QObject*))
    , this, SLOT(QObjectDestroyed(QObject*)));

  // In Reset() m_MultiWidget is used, set it to 0 now for avoiding errors
  if(_MultiWidget == 0)
    m_MultiWidget = 0;
  this->Reset();

  m_MultiWidget = _MultiWidget;

  if(m_MultiWidget == 0)
  {
    MITK_WARN << "m_MultiWidget is 0";
    this->setEnabled(false);
  }
  else
  {
    this->setEnabled(true);

    this->connect( m_MultiWidget, SIGNAL(destroyed(QObject*))
      , this, SLOT(QObjectDestroyed(QObject*)));
  }
}

QmitkStdMultiWidget* QmitkOpenCVVideoControls::GetStdMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkOpenCVVideoControls::SetVideoBackground( QmitkVideoBackground* _VideoBackground )
{
  if(m_VideoBackground == _VideoBackground)
    return;

  if(m_VideoBackground != 0)
    this->disconnect( m_VideoBackground, SIGNAL(destroyed(QObject*))
    , this, SLOT(QObjectDestroyed(QObject*)));

  this->Reset();

  m_VideoBackground = _VideoBackground;

  if(m_VideoBackground == 0)
  {
    m_VideoSource = 0;
    MITK_WARN << "m_MultiWidget is 0";
    this->setEnabled(false);
  }
  else
  {
    this->setEnabled(true);
    m_VideoSource = dynamic_cast<mitk::OpenCVVideoSource*>(m_VideoBackground->GetVideoSource());
    // preset form entries
    if(m_VideoSource != 0)
    {
      if(!m_VideoSource->GetVideoFileName().empty())
      {
        m_Controls->FileChooser->SetFile( m_VideoSource->GetVideoFileName() );
        on_UseGrabbingDeviceButton_clicked( false );
      }
      else if( m_VideoSource->GetGrabbingDeviceNumber() >= 0)
        m_Controls->GrabbingDeviceNumber->setValue( m_VideoSource->GetGrabbingDeviceNumber() );

      m_Controls->UpdateRate->setValue( m_VideoBackground->GetTimerDelay() );

      this->connect( m_VideoBackground, SIGNAL(destroyed(QObject*))
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

void QmitkOpenCVVideoControls::QObjectDestroyed( QObject * obj /*= 0 */ )
{
  if(m_MultiWidget == obj)
    this->SetStdMultiWidget(0);
  else if(m_VideoBackground == obj)
  {
    m_VideoSource = 0;
    this->SetVideoBackground(0);
  }
}
