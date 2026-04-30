/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVideoBackground_h
#define QmitkVideoBackground_h

// MITK
#include <MitkQtWidgetsExtExports.h>
#include <mitkVideoSource.h>

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
 * \brief Displays 3-channel video data as the background layer of vtkRenderWindows.
 *
 * The video frames are provided by a mitk::VideoSource via GetVideoTexture().
 * A QTimer periodically fetches frames and updates all registered render windows.
 *
 * \note The texture data is not copied; the caller must ensure the pointer
 *       returned by GetVideoTexture() remains valid. Image dimensions must
 *       be set correctly on the video source before enabling the background.
 *
 * \sa mitk::VideoSource
 */
class MITKQTWIDGETSEXT_EXPORT QmitkVideoBackground : public QObject
{
  Q_OBJECT

public:
  /**
   * \brief Construct a video background with no video source.
   *
   * You must call SetVideoSource() and AddRenderWindow() before enabling.
   *
   * \param[in] parent The parent QObject.
   */
  explicit QmitkVideoBackground(QObject *parent = nullptr);

  /**
   * \brief Construct a video background with the given video source.
   *
   * No parent is set; call setParent() or delete manually.
   * You must call AddRenderWindow() before enabling.
   *
   * \param[in] v The video source providing frames.
   * \param[in] TimerDelay Refresh interval in milliseconds (default 25 ms = 40 Hz).
   */
  explicit QmitkVideoBackground(mitk::VideoSource *v, int TimerDelay = 25);

  /** \brief Destructor. Disables all video backgrounds. */
  ~QmitkVideoBackground() override;

  /**
   * \brief Add a render window in which the video is displayed.
   *
   * Must be called before Enable(). If the render window was previously
   * added, it will be re-inserted. If the video source has no image width
   * yet, FetchFrame() will be called to obtain the first frame dimensions.
   *
   * \param[in] renderWindow The VTK render window to show video in.
   */
  void AddRenderWindow(vtkRenderWindow *renderWindow);

  /**
   * \brief Remove a render window and disable video background in it.
   * \param[in] renderWindow The VTK render window to remove.
   */
  void RemoveRenderWindow(vtkRenderWindow *renderWindow);

  /**
   * \brief Check whether a render window is currently registered.
   * \param[in] renderWindow The VTK render window to check.
   * \return True if the render window is registered.
   */
  bool IsRenderWindowIncluded(vtkRenderWindow *renderWindow);

  /**
   * \brief Set the video refresh interval.
   * \param[in] ms Timer interval in milliseconds (default 25).
   */
  void SetTimerDelay(int ms);

  /**
   * \brief Enable video background visualization and start the update timer.
   * \pre A video source and at least one render window must be set.
   */
  void Enable();

  /** \brief Disable video background visualization and stop the update timer. */
  void Disable();

  /**
   * \brief Check whether the video background is currently enabled.
   * \return True if the update timer is running.
   */
  bool IsEnabled();

  /**
   * \brief Get the attached video source.
   * \return Pointer to the mitk::VideoSource, or nullptr.
   */
  mitk::VideoSource *GetVideoSource();

  /**
   * \brief Get the current timer delay.
   * \return The timer interval in milliseconds.
   */
  int GetTimerDelay();

  /** \brief Pause the video playback by stopping the update timer. */
  void Pause();

  /** \brief Resume the video playback by restarting the update timer. */
  void Resume();

  /**
   * \brief Set a new video source.
   *
   * If a previous source was set, its observer is removed.
   * If the background was enabled, it will be stopped.
   *
   * \param[in] videoSource The new video source, or nullptr to clear.
   */
  void SetVideoSource(mitk::VideoSource *videoSource);

  /**
   * \brief Static VTK callback for render window deletion events.
   * \param[in] object The deleted VTK render window.
   * \param[in] eid The event ID.
   * \param[in] clientdata Pointer to this QmitkVideoBackground instance.
   */
  static void OnRenderWindowDelete(vtkObject *object, unsigned long eid, void *clientdata, void * /*calldata*/);

  /**
   * \brief ITK observer callback for video source deletion events.
   * \param[in] caller The deleted video source.
   * \param[in] event The ITK event object.
   */
  void OnVideoSourceDelete(const itk::Object *caller, const itk::EventObject &event);

public slots:
  /** \brief Fetch a new video frame and update all registered render windows. */
  void UpdateVideo();

signals:
  /**
   * \brief Emitted after all render windows have been updated with a new frame.
   * \param[in] videoSource The video source that provided the frame.
   */
  void NewFrameAvailable(mitk::VideoSource *videoSource);

  /**
   * \brief Emitted when the video source reaches the end of its stream.
   * \param[in] videoSource The video source that ended.
   */
  void EndOfVideoSourceReached(mitk::VideoSource *videoSource);

protected:
  ///
  /// class for holding all vtk dependencies
  /// needed to do background image rendering
  ///
  struct VideoBackgroundVectorInfo
  {
    vtkRenderWindow *renWin;
    vtkRenderer *videoRenderer;
    vtkImageActor *videoActor;
    vtkImageImport *videoImport;
    unsigned long renderWindowObserverTag;
  };

  ///
  /// removes the renderwindow and also removes the observer if the flag is set
  ///
  void RemoveRenderWindow(vtkRenderWindow *renderWindow, bool removeObserver);
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
  QTimer *m_QTimer;

  ///
  /// must implement GetVideoTexture() correctly (must return an OpenGL texture)
  ///
  mitk::VideoSource *m_VideoSource;

  ///
  /// the observer tag for the video source
  ///
  unsigned long m_VideoSourceObserverTag;
};

#endif
