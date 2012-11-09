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

#ifndef _Qmitk_Video_Background_h_
#define _Qmitk_Video_Background_h_

// MITK
#include "mitkVideoSource.h"
#include "QmitkExtExports.h"

// Qt
#include <QObject>
class QTimer;

// vtk
class vtkRenderer;
class vtkRenderWindow;
class vtkImageActor;
class vtkImageImport;
class vtkActor2D;
class vtkVideoSizeCallback;
class vtkObject;

/**
 * Displays a 3-channel (!) video data in the background
 * of one or more vtkRenderWindow(s).
 * The video is provided by a mitkVideoSource / GetVideoTexture().
 * Caution: As the texture data is not being copied, the user is responsible for a valid
 * pointer to the data. Also the image dimensions needs to be set correctly before enabling the
 * background.
 */
class QmitkExt_EXPORT QmitkVideoBackground : public QObject
{
  Q_OBJECT

public:
  ///
  /// default ctor, TimerDelay is 40 by default
  /// you must SetVideoSource() and AddRenderWindow() afterwards
  ///
  explicit QmitkVideoBackground(QObject *parent=0);
  ///
  /// constructs a video background with the given video source
  /// no parent is set here, dont forget to delete the object or
  /// call setParent()
  /// TimerDelay = refresh rate of video in ms (25 ms = 40 Hz).
  /// you must call AddRenderWindow() afterwards
  ///
  explicit QmitkVideoBackground(mitk::VideoSource* v, int TimerDelay = 25);
  ///
  /// disables all video backgrounds
  ///
  virtual ~QmitkVideoBackground();

  ///
  /// \brief add a RenderWindow in which the video is displayed.
  /// -> must be initialized before enabling the background.
  /// if the renderwindow was previously inserted it will get
  /// re-inserted (restarted videobackground)
  /// *ATTENTION*: to size the renderwindow correctly GetImageWidth() of the video
  /// source will be called and if *no size is returned: FetchFrame()
  /// on the video source will be called to get the first frame and
  /// the corresponding size*
  ///
  void AddRenderWindow(vtkRenderWindow* renderWindow);
  ///
  /// \brief removes a renderwindow = disables video background there
  ///
  void RemoveRenderWindow(vtkRenderWindow* renderWindow);
  ///
  /// \return true if "renderWindow" is currently connected to the video
  /// background or not
  ///
  bool IsRenderWindowIncluded(vtkRenderWindow* renderWindow);

  ///
  /// \brief sets the update rate of the video in milli seconds, by default 25.
  ///
  void SetTimerDelay(int ms);
  ///
  /// visualizes the video. Requires image dimensions and an active videosource to be set.
  ///
  void Enable();
  ///
  /// \brief disables visualization of the video.
  ///
  void Disable();
  ///
  /// \brief Checks, if the Video background is currently enabled (visible).
  ///
  bool IsEnabled();

  ///
  /// Returns the videosource attached to this background
  ///
  mitk::VideoSource* GetVideoSource();
  ///
  /// Returns the timer delay
  ///
  int GetTimerDelay();
  ///
  /// pauses the playback (stops the update timer)
  ///
  void Pause();
  ///
  /// resumes the playback (restarts the update timer)
  ///
  void Resume();

  ///
  /// sets a *new* video source (if previously enabled, this will stop
  /// the video background if it was previously enabled
  ///
  void SetVideoSource( mitk::VideoSource* videoSource );

  ///
  /// receive renderwindow delete events
  ///
  static void OnRenderWindowDelete(vtkObject *, unsigned long eid , void* clientdata, void * /*calldata*/);

  ///
  /// receive VideoSource delete event
  ///
  void OnVideoSourceDelete(const itk::Object* caller
    , const itk::EventObject &event);
  public slots:
    ///
    /// update all video backgrounds. (called by the timer or manually
    /// by the user)
    ///
    void UpdateVideo();
  signals:
    ///
    /// emitted after all video backgrounds are filled with the new
    /// video frame
    ///
    void NewFrameAvailable(mitk::VideoSource*);

protected:
  ///
  /// class for holding all vtk dependencies
  /// needed to do background image rendering
  ///
  struct VideoBackgroundVectorInfo
  {
    vtkRenderWindow*  renWin;
    vtkRenderer*      videoRenderer;
    vtkImageActor*    videoActor;
    vtkImageImport*   videoImport;
    unsigned long renderWindowObserverTag;
  };

  ///
  /// removes the renderwindow and also removes the observer if the flag is set
  ///
  void RemoveRenderWindow( vtkRenderWindow* renderWindow, bool removeObserver );
  ///
  /// reset all video backgrounds
  ///
  void ResetVideoBackground();

  ///
  /// inits all renderwindows with default values, called before video rendering is started
  ///
  void Modified();
  ///
  /// the class has to store a list of renderwindows
  ///
  typedef std::vector<VideoBackgroundVectorInfo> RenderWindowVectorInfoType;

protected:
  ///
  /// a list of renderwindows and associated renderers and actors and imageimporters
  ///
  RenderWindowVectorInfoType m_renderWindowVectorInfo;

  ///
  /// calls updatevideo repeateadly for framegrabbing
  ///
  QTimer* m_QTimer;

  ///
  /// must implement GetVideoTexture() correctly (must return an OpenGL texture)
  ///
  mitk::VideoSource* m_VideoSource;

  ///
  /// the observer tag for the video source
  ///
  unsigned long m_VideoSourceObserverTag;
};

#endif

