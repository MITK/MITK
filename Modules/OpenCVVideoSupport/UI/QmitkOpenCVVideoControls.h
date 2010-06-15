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

#ifndef QmitkOpenCVVideoControls_h
#define QmitkOpenCVVideoControls_h

#include <cv.h>
#include <ui_QmitkOpenCVVideoControls.h>
#include <mitkOpenCVVideoSupportUIExports.h>

class QmitkStdMultiWidget;
class QmitkVideoBackground;
namespace mitk
{
  class VideoSource;
  class OpenCVVideoSource;
}

///
/// \brief Offers widgets to play/pause/stop a video on a certain render window with
/// the use of an !initialized! QmitkVideoBackground. The QmitkVideoBackground should
/// contain an OpenCVVideoSource is then owned by this widget (and deleted)
///
class MITK_OPENCVVIDEOSUPPORTUI_EXPORT QmitkOpenCVVideoControls : public QWidget
{
  Q_OBJECT

public:
  ///
  /// Construct the widget with the given render window and the given preset values
  ///
  QmitkOpenCVVideoControls(QmitkVideoBackground* _VideoBackground, QmitkStdMultiWidget* _MultiWidget
    , QWidget * parent = 0, Qt::WindowFlags f = 0);

  ///
  /// call reset if video playback is enabled here
  ///
  virtual ~QmitkOpenCVVideoControls();

  ///
  /// sets the multiwidget for this video player
  ///
  void SetStdMultiWidget(QmitkStdMultiWidget* _MultiWidget);

  ///
  /// returns the current multiwidget
  ///
  QmitkStdMultiWidget* GetStdMultiWidget() const;

  ///
  /// sets the qmitkvideobackground for this
  ///
  void SetVideoBackground(QmitkVideoBackground* _VideoBackground);

  ///
  /// returns the current QmitkVideoBackground
  ///
  QmitkVideoBackground* GetVideoBackground() const;
signals:
  ///
  /// When playback is started this informs when a new frame was grabbed
  ///
  void NewOpenCVFrameAvailable(const IplImage*);

protected slots:
  void on_UseGrabbingDeviceButton_clicked(bool checked=false);
  void on_UseVideoFileButton_clicked(bool checked=false);
  void on_VideoProgressSlider_sliderPressed();
  void on_VideoProgressSlider_sliderReleased();
  void on_RepeatVideoButton_clicked( bool checked=false );
  void on_PlayButton_clicked(bool checked=false);
  void on_StopButton_clicked(bool checked=false);
  void Stop();
  void Reset();
  void SwitchPlayButton(bool paused);
  void QObjectDestroyed( QObject * obj = 0 );

  void NewFrameAvailable(mitk::VideoSource* videoSource);
protected:
  QmitkVideoBackground* m_VideoBackground;
  QmitkStdMultiWidget* m_MultiWidget;
  mitk::OpenCVVideoSource* m_VideoSource;
  Ui::QmitkOpenCVVideoControls* m_Controls;
  bool m_SliderCurrentlyMoved;
};

#endif
