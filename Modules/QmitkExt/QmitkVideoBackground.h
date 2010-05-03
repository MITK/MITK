/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _Qmitk_Video_Background_h_
#define _Qmitk_Video_Background_h_

#include "mitkVideoSource.h"
#include "QmitkExtExports.h"

// Q-Timer
#include "qobject.h"
class QTimer;
class QTime;

class vtkRenderer;
class vtkRenderWindow;
class vtkImageActor;
class vtkImageImport;
class vtkActor2D;
class vtkVideoSizeCallback;


namespace mitk 
{

  struct VideoBackgroundVectorInfo
  {
    vtkRenderWindow*  renWin;
    vtkRenderer*      videoRenderer;
    vtkImageActor*    videoActor;
    vtkImageImport*   videoImport;
  };

}

/**
 * Displays a video in the background
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
  QmitkVideoBackground(mitk::VideoSource* v, int TimerDelay = 25); // TimerDelay = refresh rate of video in ms.
   ~QmitkVideoBackground();

  ////##Documentation
  ////## @brief sets the mitkRenderWindow in which the video is displayed.
  ////## must be initialized before enabling the background.
  void AddRenderWindow(vtkRenderWindow* renderWindow);
  void RemoveRenderWindow(vtkRenderWindow* renderWindow);
  bool IsRenderWindowIncluded(vtkRenderWindow* renderWindow);

  ////##Documentation
  ////## @brief sets the update rate of the video in milli seconds, by default 25.
  void SetTimerDelay(int ms)
  {
    m_TimerDelay = ms;
    ResetVideoBackground();
  }
  ////##Documentation
  ////## @brief visualizes the video. Requires image dimensions and an active mitkvideosource to be set.
  void Enable();
  ////##Documentation
  ////## @brief disables visualization of the video.
  void Disable();
  ////##Documentation
  ////## @brief Checks, if the Video background is currently enabled (visible).
  bool IsEnabled();

  ///
  /// Returns the videosource attached to this background
  ///
  mitk::VideoSource* GetVideoSource() { return m_VideoSource; }
  ///
  /// Returns the timer delay
  ///
  int GetTimerDelay() { return m_TimerDelay; }
  
  public slots:
    void UpdateVideo();
  signals:
    void NewFrameAvailable(mitk::VideoSource*);

protected:
  void ResetVideoBackground();

  void Modified();

  typedef std::vector<mitk::VideoBackgroundVectorInfo> RenderWindowVectorInfoType;
  RenderWindowVectorInfoType      m_renderWindowVectorInfo;
 
  //for framegrabbing
  QTimer*                     m_QTimer;

  //VideoSource
  mitk::VideoSource*          m_VideoSource;

  int m_TimerDelay;
};

#endif

