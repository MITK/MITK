#ifndef _Qmitk_Video_Background_h_
#define _Qmitk_Video_Background_h_

#include "mitkVideoSource.h"

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


namespace mitk {
class RenderWindow;
}
/**
 * Displays a video in the background
 * of a vtkRenderWindow.
 */
class QmitkVideoBackground : public QObject
{
  Q_OBJECT

public:
  QmitkVideoBackground(mitk::VideoSource* v);
   ~QmitkVideoBackground();

  mitk::RenderWindow* GetRenderWindow();
  void SetRenderWindow(mitk::RenderWindow* renderWindow);

  void Enable();
  void Disable();

  void SetImageDimensions(int x, int y)
  {
    m_ImageWidth  = x;
    m_ImageHeight = y;
    ResetVideoBackground();
  }

  void SetTimerDelay(int ms)
  {
    m_TimerDelay = ms;
    ResetVideoBackground();
  }
  
   /**
   * Checks, if the Video background is currently
   * enabled (visible)
   */
   bool IsEnabled();
  
  public slots:
    void UpdateVideo();
  
protected:
  void ResetVideoBackground();

  mitk::RenderWindow*         m_RenderWindow;
  vtkRenderer*                m_VideoRenderer;
  vtkImageActor*              m_Actor;
  vtkVideoSizeCallback*       m_VideoCallback;
  vtkImageImport*             m_VtkImageImport;
 
  //for framegrabbing
  QTimer*                     m_QTimer;

  //VideoSource
  mitk::VideoSource*            m_VideoSource;

  int m_ImageWidth, m_ImageHeight, m_TimerDelay;
};


#endif

