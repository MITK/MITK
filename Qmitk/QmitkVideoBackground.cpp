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

#include "QmitkVideoBackground.h"
// MITK includes
#include "mitkVtkLayerController.h"
// QT includes
#include "qtimer.h"
// VTK includes
#include "vtkSystemIncludes.h"
#include "vtkRenderer.h"
#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkImageActor.h"
#include "vtkRenderWindow.h"
#include "vtkImageImport.h"
#include "vtkCommand.h"
#include "vtkCamera.h"


// VTK CALLBACK for automatic resize of the video
class vtkVideoSizeCallback : public vtkCommand
{
public:
  static vtkVideoSizeCallback *New(){ return new vtkVideoSizeCallback; }
  
  vtkRenderer * m_VideoRenderer;
  int m_ImageWidth, m_ImageHeight;

  void SetVtkVideoRenderer(vtkRenderer* r)
  {
    m_VideoRenderer = r;
  }
  void SetVideoDimensions(int x, int y)
  {
    m_ImageWidth = x; m_ImageHeight = y;
  }

  virtual void Execute(vtkObject * /*caller*/, unsigned long, void*)
  {
    /*vtkRenderWindow * RenderWindow = reinterpret_cast<vtkRenderWindow*>(caller);
    int * windowSize = RenderWindow->GetScreenSize();
    int horRatio = windowSize[0]/m_ImageWidth;
    int verRatio = windowSize[1]/m_ImageHeight;*/

    //if(horRatio < verRatio)
    //  m_VideoRenderer->GetActiveCamera()->SetParallelScale(m_ImageWidth/2);
    //else
    //  m_VideoRenderer->GetActiveCamera()->SetParallelScale(m_ImageHeight/2);
    //
    //std::cout << "video size adjusted" << std::endl;
    //m_VideoRenderer->ResetCameraClippingRange();
  }
};

QmitkVideoBackground::QmitkVideoBackground(mitk::VideoSource* v)
{
  m_VideoSource = v; 
  m_ImageWidth  = 720;
  m_ImageHeight = 576;
  m_TimerDelay  = 50;
  ResetVideoBackground();
}
void QmitkVideoBackground::ResetVideoBackground()
{
  m_RenderWindow = NULL;
  m_QTimer = new QTimer(this, "VideoTimer");
  connect( m_QTimer, SIGNAL(timeout()), SLOT(UpdateVideo()) );
  
  m_Actor = vtkImageActor::New();
  m_VideoRenderer = vtkRenderer::New();

  m_VtkImageImport = vtkImageImport::New();
  m_VtkImageImport->SetDataScalarTypeToUnsignedChar();
  m_VtkImageImport->SetNumberOfScalarComponents(3);
  m_VtkImageImport->SetWholeExtent(0,m_ImageWidth-1,0,m_ImageHeight-1,0,1-1);
  m_VtkImageImport->SetDataExtentToWholeExtent();  
}

QmitkVideoBackground::~QmitkVideoBackground()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  if ( m_Actor!=NULL )
    m_Actor->Delete();
  if ( m_VideoRenderer != NULL )
    m_VideoRenderer->Delete();
  if ( m_VtkImageImport != NULL)
    m_VtkImageImport->Delete();
}
/**
 * Sets the renderwindow, in which the Video background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void QmitkVideoBackground::SetRenderWindow(vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;

  m_VideoCallback = vtkVideoSizeCallback::New();
  //m_RenderWindow->AddObserver(vtkCommand::ModifiedEvent,m_VideoCallback);
}
/**
 * Enables drawing of the color Video background.
 * If you want to disable it, call the Disable() function.
 */
void QmitkVideoBackground::Enable()
{
  m_VideoRenderer = vtkRenderer::New();
   
  UpdateVideo();
  m_Actor->SetInput(m_VtkImageImport->GetOutput());
 
  m_VideoRenderer->AddActor2D(m_Actor);
  m_VideoRenderer->ResetCamera();
  m_VideoRenderer->InteractiveOff();
  m_VideoRenderer->GetActiveCamera()->ParallelProjectionOn();
  m_VideoRenderer->GetActiveCamera()->SetParallelScale(m_ImageHeight/2);
 
  m_VideoCallback->SetVtkVideoRenderer(m_VideoRenderer);
  m_VideoCallback->SetVideoDimensions(m_ImageWidth, m_ImageHeight);
  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertBackgroundRenderer(m_VideoRenderer,true);
  m_QTimer->start(m_TimerDelay);
  
}

/**
 * Disables drawing of the color Video background.
 * If you want to enable it, call the Enable() function.
 */
void QmitkVideoBackground::Disable()
{
  if ( this->IsEnabled() )
  {
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_VideoRenderer);
    m_QTimer->stop();
  }
}
/**
 * Checks, if the Video background is currently
 * enabled (visible)
 */
bool QmitkVideoBackground::IsEnabled()
{
  if (mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted(m_VideoRenderer))
      return true;
  else
      return false;    
}

void QmitkVideoBackground::UpdateVideo()
{  
  unsigned char *src = 0;
  src = m_VideoSource->GetVideoTexture();
  if(src)
  {
    m_VtkImageImport->SetImportVoidPointer(src);
    m_VtkImageImport->Modified();
    m_VtkImageImport->Update();
    m_RenderWindow->Render();
  } 
}

 // how to superimpose a logo
 // vtkCamera*          camera2        = vtkCamera::New();
 // vtkImageMapper   *   mapper2D      = vtkImageMapper::New();
 // vtkPNGReader      *  pngReader   = vtkPNGReader::New();
 // vtkActor2D      *    actor2D     = vtkActor2D::New();
 // pngReader->SetFileName("D:\\mbilogo.png");
 // pngReader->Update();
 // mapper2D->SetInput(pngReader->GetOutput());
 // double range[2];
 // pngReader->GetOutput()->GetScalarRange(range);
 // mapper2D->SetColorWindow(range[1] - range[0] );
 // mapper2D->SetColorLevel((range[0]+range[1])/2);
 // actor2D->SetPosition(0,0);
 ///* actor2D->SetWidth(300);
 // actor2D->SetHeight(200);*/
 // actor2D->SetMapper(mapper2D);
 // actor2D->SetLayerNumber(99);
 // m_RenderWindow->GetVtkLayerController()->GetSceneRenderer()->AddActor2D(actor2D);
  /*renderer2->SetActiveCamera(camera);
  renderer2->ResetCamera();
  m_RenderWindow->GetVtkLayerController()->InsertForegroundRenderer(renderer2,true);*/
