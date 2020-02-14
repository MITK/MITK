/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOpenCVVideoControls_h
#define QmitkOpenCVVideoControls_h

#include <ui_QmitkOpenCVVideoControls.h>
#include <MitkOpenCVVideoSupportUIExports.h>
#include <mitkPropertyListReplacedObserver.h>

#include "opencv2/core.hpp"

class QmitkRenderWindow;
class QmitkVideoBackground;
namespace mitk
{
  class VideoSource;
  class OpenCVVideoSource;
}

class QmitkOpenCVVideoControlsPrivate;

///
/// \brief Offers widgets to play/pause/stop a video on a certain render window with
/// the use of an !initialized! QmitkVideoBackground. The QmitkVideoBackground should
/// contain an OpenCVVideoSource is then owned by this widget (and deleted)
///
class MITKOPENCVVIDEOSUPPORTUI_EXPORT QmitkOpenCVVideoControls : public QWidget, public mitk::PropertyListReplacedObserver
{
  Q_OBJECT

public:
  ///
  /// Construct the widget with the given render window and the given preset values
  ///
  QmitkOpenCVVideoControls(QmitkVideoBackground* _VideoBackground, QmitkRenderWindow* _RenderWindow
    , QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

  ///
  /// call reset if video playback is enabled here
  ///
  ~QmitkOpenCVVideoControls() override;

  ///
  /// sets the render window for this video player
  ///
  void SetRenderWindow(QmitkRenderWindow* _RenderWindow);

  ///
  /// returns the current render window
  ///
  QmitkRenderWindow* GetRenderWindow() const;

  ///
  /// sets the qmitkvideobackground for this
  ///
  void SetVideoBackground(QmitkVideoBackground* _VideoBackground);

  ///
  /// returns the current QmitkVideoBackground
  ///
  QmitkVideoBackground* GetVideoBackground() const;

  ///
  /// calls FromPropertyList
  ///
  void AfterPropertyListReplaced(const std::string& id, mitk::PropertyList* propertyList) override;

signals:
  ///
  /// When playback is started this informs when a new frame was grabbed
  ///
  void NewOpenCVFrameAvailable(const IplImage*);

  protected slots:
  void on_UseGrabbingDeviceButton_clicked(bool checked = false);
  void on_UseVideoFileButton_clicked(bool checked = false);
  void on_VideoProgressSlider_sliderPressed();
  void on_VideoProgressSlider_sliderReleased();
  void on_VideoProgressSlider_valueChanged(int value);
  void on_RepeatVideoButton_clicked(bool checked = false);
  void on_PlayButton_clicked(bool checked = false);
  void on_StopButton_clicked(bool checked = false);
  void Play();
  void Stop();
  void Reset();
  void IsPlaying(bool paused);
  void QObjectDestroyed(QObject * obj = nullptr);

  void NewFrameAvailable(mitk::VideoSource* videoSource);
  void EndOfVideoSourceReached(mitk::VideoSource* videoSource);

protected:
  QmitkVideoBackground* m_VideoBackground;
  QmitkRenderWindow* m_RenderWindow;
  mitk::OpenCVVideoSource* m_VideoSource;
  Ui::QmitkOpenCVVideoControls* m_Controls;
  bool m_SliderCurrentlyMoved;

private:

  friend class QmitkOpenCVVideoControlsPrivate;

  QScopedPointer<QmitkOpenCVVideoControlsPrivate> d;
};

#endif
