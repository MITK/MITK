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

#ifndef QmitkOpenCVVideoControls_h
#define QmitkOpenCVVideoControls_h

#include <cv.h>
#include <ui_QmitkOpenCVVideoControls.h>
#include <MitkOpenCVVideoSupportUIExports.h>
#include <mitkPropertyListReplacedObserver.h>

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
class MITK_OPENCVVIDEOSUPPORTUI_EXPORT QmitkOpenCVVideoControls : public QWidget,  public mitk::PropertyListReplacedObserver
{
  Q_OBJECT

public:
  ///
  /// Construct the widget with the given render window and the given preset values
  ///
  QmitkOpenCVVideoControls(QmitkVideoBackground* _VideoBackground, QmitkRenderWindow* _RenderWindow
    , QWidget * parent = 0, Qt::WindowFlags f = 0);

  ///
  /// call reset if video playback is enabled here
  ///
  virtual ~QmitkOpenCVVideoControls();

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
  void AfterPropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList );

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
  QmitkRenderWindow* m_RenderWindow;
  mitk::OpenCVVideoSource* m_VideoSource;
  Ui::QmitkOpenCVVideoControls* m_Controls;
  bool m_SliderCurrentlyMoved;

private:

  friend class QmitkOpenCVVideoControlsPrivate;

  QScopedPointer<QmitkOpenCVVideoControlsPrivate> d;

};

#endif
