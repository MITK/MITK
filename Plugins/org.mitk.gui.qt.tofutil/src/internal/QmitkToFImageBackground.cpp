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

#include "QmitkToFImageBackground.h"
// MITK includes
#include "mitkVtkLayerController.h"

#include "mitkRenderingManager.h"

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

QmitkToFImageBackground::QmitkToFImageBackground()
{
  ResetBackground();
}
void QmitkToFImageBackground::ResetBackground()
{
  m_renderWindowVectorInfo.clear();
}

QmitkToFImageBackground::~QmitkToFImageBackground()
{
  if ( m_renderWindowVectorInfo.size() > 0 )
  {
  }
}
/**
 * Sets the renderwindow, in which the Video background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
 /*
void QmitkToFImageBackground::AddRenderWindow(vtkRenderWindow* renderWindow )
{
  RemoveRenderWindow(renderWindow);

  vtkRenderer*    videoRenderer   = vtkRenderer::New();
  vtkImageActor*  videoActor      = vtkImageActor::New();
  vtkImageImport* videoImport     = vtkImageImport::New();

  videoImport->SetDataScalarTypeToUnsignedChar();
  videoImport->SetNumberOfScalarComponents(3);

  if(m_VideoSource->GetImageWidth() == 0)
    m_VideoSource->FetchFrame();

  videoImport->SetWholeExtent(0,m_VideoSource->GetImageWidth()-1,0,m_VideoSource->GetImageHeight()-1,0,1-1);
  videoImport->SetDataExtentToWholeExtent();

  mitk::VideoBackgroundVectorInfo v;
  v.renWin        = renderWindow;
  v.videoRenderer = videoRenderer;
  v.videoActor    = videoActor;
  v.videoImport   = videoImport;

  m_renderWindowVectorInfo.push_back(v);

  Modified();
}
*/
void QmitkToFImageBackground::AddRenderWindow(vtkRenderWindow* renderWindow, int width, int height )
{
  RemoveRenderWindow(renderWindow);

  vtkRenderer*    videoRenderer   = vtkRenderer::New();
  vtkImageActor*  videoActor      = vtkImageActor::New();
  vtkImageImport* videoImport     = vtkImageImport::New();

  videoImport->SetDataScalarTypeToUnsignedChar();
  videoImport->SetNumberOfScalarComponents(3);

//  if(m_VideoSource->GetImageWidth() == 0)
//    m_VideoSource->FetchFrame();

  videoImport->SetWholeExtent(0,width-1,0,height-1,0,1-1);
  videoImport->SetDataExtentToWholeExtent();

  mitk::ToFImageBackgroundVectorInfo v;
  v.renWin        = renderWindow;
  v.videoRenderer = videoRenderer;
  v.videoActor    = videoActor;
  v.videoImport   = videoImport;

  m_renderWindowVectorInfo.push_back(v);

  //Modified();
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    (*it).videoActor->SetInput((*it).videoImport->GetOutput());
    (*it).videoRenderer->AddActor2D((*it).videoActor);
    (*it).videoRenderer->ResetCamera();
    (*it).videoRenderer->InteractiveOff();
    (*it).videoRenderer->GetActiveCamera()->ParallelProjectionOn();
    (*it).videoRenderer->GetActiveCamera()->SetParallelScale(height/2);

    if(!mitk::VtkLayerController::GetInstance((*it).renWin)->IsRendererInserted((*it).videoRenderer))
      mitk::VtkLayerController::GetInstance((*it).renWin)->InsertBackgroundRenderer((*it).videoRenderer,true);
  }
}

void QmitkToFImageBackground::RemoveRenderWindow(vtkRenderWindow* renderWindow )
{
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    if((*it).renWin == renderWindow)
    {
      // unregister video backround renderer from renderwindow
      mitk::VtkLayerController::GetInstance((*it).renWin)->RemoveRenderer((*it).videoRenderer);

      (*it).videoRenderer->Delete();
      (*it).videoActor->Delete();
      (*it).videoImport->Delete();
      m_renderWindowVectorInfo.erase(it);
      return;
      //delete &(*it);  // memory leak renderwindowvectorinfo ??
    }
  }
}

bool QmitkToFImageBackground::IsRenderWindowIncluded(vtkRenderWindow* renderWindow )
{
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    if((*it).renWin == renderWindow)
      return true;
  }
  return false;
}

/**
 * Enables drawing of the color Video background.
 * If you want to disable it, call the Disable() function.
 */
 /*
void QmitkToFImageBackground::Enable()
{
  UpdateVideo();
  Modified();

  m_QTimer->start(m_TimerDelay);
}
*/
/**
 * Disables drawing of the color Video background.
 * If you want to enable it, call the Enable() function.
 */
/*
void QmitkToFImageBackground::Disable()
{
  if ( this->IsEnabled() )
  {
    /*for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
      it != m_renderWindowVectorInfo.end(); it++)
    {
      mitk::VtkLayerController::GetInstance((*it).renWin)->RemoveRenderer((*it).videoRenderer);
    }* /
    m_QTimer->stop();
  }
}
*/
/**
 * Checks, if the Video background is currently
 * enabled (visible)
 */
 /*
bool QmitkToFImageBackground::IsEnabled()
{
  if (m_QTimer->isActive())
      return true;
  else
      return false;
}
*/
/*
void QmitkToFImageBackground::UpdateVideo()
{
  unsigned char *src = 0;
  src = m_VideoSource->GetVideoTexture();
  if(src)
  {
    if(m_renderWindowVectorInfo.size()>0)
    {
      for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
        it != m_renderWindowVectorInfo.end(); it++)
      {
        (*it).videoImport->SetImportVoidPointer(src);
        (*it).videoImport->Modified();
        (*it).videoImport->Update();
        mitk::RenderingManager::GetInstance()->RequestUpdate((*it).renWin);
      }
    }
  }
  emit NewFrameAvailable ( m_VideoSource );
}
*/
void QmitkToFImageBackground::UpdateBackground(unsigned char *src)
{
  if(src)
  {
    if(m_renderWindowVectorInfo.size()>0)
    {
      for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
        it != m_renderWindowVectorInfo.end(); it++)
      {
        (*it).videoImport->SetImportVoidPointer(src);
        (*it).videoImport->Modified();
        (*it).videoImport->Update();
        mitk::RenderingManager::GetInstance()->RequestUpdate((*it).renWin);
      }
    }
  }
  //emit NewFrameAvailable ( m_VideoSource );
}

void QmitkToFImageBackground::Modified()
{ // ensures registration of video backrounds in each renderwindow
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    (*it).videoActor->SetInput((*it).videoImport->GetOutput());
    (*it).videoRenderer->AddActor2D((*it).videoActor);
    (*it).videoRenderer->ResetCamera();
    (*it).videoRenderer->InteractiveOff();
    (*it).videoRenderer->GetActiveCamera()->ParallelProjectionOn();
    //(*it).videoRenderer->GetActiveCamera()->SetParallelScale(m_VideoSource->GetImageHeight()/2);

    if(!mitk::VtkLayerController::GetInstance((*it).renWin)->IsRendererInserted((*it).videoRenderer))
      mitk::VtkLayerController::GetInstance((*it).renWin)->InsertBackgroundRenderer((*it).videoRenderer,true);
  }
}
