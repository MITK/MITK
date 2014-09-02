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

#include "QmitkVideoBackground.h"

// MITK includes
#include "mitkVtkLayerController.h"
#include "mitkRenderingManager.h"

// QT includes
#include <QTimer>

// itk includes
#include <itkCommand.h>

// VTK includes
#include <vtkSystemIncludes.h>
#include <vtkRenderer.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkImageImport.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

QmitkVideoBackground::QmitkVideoBackground( QObject *parent )
: QObject(parent)
, m_QTimer(new QTimer(this))
, m_VideoSource(0)
, m_VideoSourceObserverTag(0)
{
  this->ResetVideoBackground();
}

QmitkVideoBackground::QmitkVideoBackground(mitk::VideoSource* v, int)
: QObject(0)
, m_QTimer(new QTimer(this))
, m_VideoSource(0)
, m_VideoSourceObserverTag(0)
{
  this->SetVideoSource( v );
  this->ResetVideoBackground();
}

void QmitkVideoBackground::ResetVideoBackground()
{
  m_QTimer->setInterval(25);
  connect( m_QTimer, SIGNAL(timeout()), SLOT(UpdateVideo()) );
  m_renderWindowVectorInfo.clear();
}

QmitkVideoBackground::~QmitkVideoBackground()
{
  this->Disable();
}

void QmitkVideoBackground::AddRenderWindow(vtkRenderWindow* renderWindow )
{
  if(!renderWindow || !m_VideoSource)
  {
    MITK_WARN << "No Renderwindow or VideoSource set!";
    return;
  }

  this->RemoveRenderWindow(renderWindow);

  vtkRenderer*    videoRenderer   = vtkRenderer::New();
  vtkImageActor*  videoActor      = vtkImageActor::New();
  vtkImageImport* videoImport     = vtkImageImport::New();

  videoImport->SetDataScalarTypeToUnsignedChar();
  videoImport->SetNumberOfScalarComponents(3);

  if(m_VideoSource->GetImageWidth() == 0)
    m_VideoSource->FetchFrame();

  videoImport->SetWholeExtent(0, m_VideoSource->GetImageWidth()-1,
    0, m_VideoSource->GetImageHeight()-1, 0, 1-1);
  videoImport->SetDataExtentToWholeExtent();

  VideoBackgroundVectorInfo v;
  v.renWin        = renderWindow;
  v.videoRenderer = videoRenderer;
  v.videoActor    = videoActor;
  v.videoImport   = videoImport;

  // callback for the deletion of the renderwindow
  vtkSmartPointer<vtkCallbackCommand> deleteCallback =
    vtkSmartPointer<vtkCallbackCommand>::New();
  deleteCallback->SetCallback ( QmitkVideoBackground::OnRenderWindowDelete );
  deleteCallback->SetClientData(this);

  v.renderWindowObserverTag = renderWindow->AddObserver( vtkCommand::DeleteEvent, deleteCallback );

  m_renderWindowVectorInfo.push_back(v);

  // completes the initialization
  this->Modified();
}

void QmitkVideoBackground::RemoveRenderWindow( vtkRenderWindow* renderWindow )
{
  this->RemoveRenderWindow(renderWindow, true);
}

void QmitkVideoBackground::RemoveRenderWindow( vtkRenderWindow* renderWindow, bool removeObserver )
{
  // search for renderwindow and remove it
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    if((*it).renWin == renderWindow)
    {
      mitk::VtkLayerController* layerController =
        mitk::VtkLayerController::GetInstance((*it).renWin);
      // unregister video backround renderer from renderwindow
      if( layerController )
        layerController->RemoveRenderer((*it).videoRenderer);

      (*it).videoRenderer->Delete();
      (*it).videoActor->Delete();
      (*it).videoImport->Delete();
      // remove listener
      if(removeObserver)
        renderWindow->RemoveObserver( (*it).renderWindowObserverTag );
      m_renderWindowVectorInfo.erase(it);
      break;
    }
  }
}

bool QmitkVideoBackground::IsRenderWindowIncluded(vtkRenderWindow* renderWindow )
{
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    if((*it).renWin == renderWindow)
      return true;
  }
  return false;
}

void QmitkVideoBackground::Pause()
{
  m_QTimer->stop();
}

void QmitkVideoBackground::Resume()
{
  m_QTimer->start();
}

/**
 * Enables drawing of the color Video background.
 * If you want to disable it, call the Disable() function.
 */
void QmitkVideoBackground::Enable()
{
  UpdateVideo();
  Modified();

  m_QTimer->start();
}

/**
 * Disables drawing of the color Video background.
 * If you want to enable it, call the Enable() function.
 */
void QmitkVideoBackground::Disable()
{
  if ( this->IsEnabled() )
  {
    mitk::VtkLayerController* layerController = 0;
    for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
      it != m_renderWindowVectorInfo.end(); it++)
    {
      layerController = mitk::VtkLayerController::GetInstance((*it).renWin);
      if(layerController)
        layerController->RemoveRenderer((*it).videoRenderer);
    }
    m_QTimer->stop();
  }
}

bool QmitkVideoBackground::IsEnabled()
{
  return m_QTimer->isActive();
}

void QmitkVideoBackground::UpdateVideo()
{
  if( m_renderWindowVectorInfo.size() > 0 )
  {
    unsigned char *src = 0;
    src = m_VideoSource->GetVideoTexture();
    if(src)
    {
      for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
        it != m_renderWindowVectorInfo.end(); it++)
      {
        (*it).videoImport->SetImportVoidPointer(src);
        (*it).videoImport->Modified();
        (*it).videoImport->Update();
        mitk::RenderingManager::GetInstance()->RequestUpdate((*it).renWin);
      }
      emit NewFrameAvailable ( m_VideoSource );
    }
    else
      MITK_WARN << "No video texture available";
  }
}

void QmitkVideoBackground::Modified()
{
  // ensures registration of video backrounds in each renderwindow
  for(RenderWindowVectorInfoType::iterator it = m_renderWindowVectorInfo.begin();
    it != m_renderWindowVectorInfo.end(); it++)
  {
    (*it).videoImport->Update();
    (*it).videoActor->SetInputData((*it).videoImport->GetOutput());
    (*it).videoRenderer->AddActor2D((*it).videoActor);
    (*it).videoRenderer->ResetCamera();
    (*it).videoRenderer->InteractiveOff();
    (*it).videoRenderer->GetActiveCamera()->ParallelProjectionOn();
    (*it).videoRenderer->GetActiveCamera()->SetParallelScale(m_VideoSource->GetImageHeight()/2);

    mitk::VtkLayerController* layerController =
      mitk::VtkLayerController::GetInstance((*it).renWin);

    if( layerController && !layerController->IsRendererInserted((*it).videoRenderer) )
      layerController->InsertBackgroundRenderer((*it).videoRenderer,true);
  }
}

void QmitkVideoBackground::SetVideoSource( mitk::VideoSource* videoSource )
{
  if( m_VideoSource == videoSource )
    return;

  if( m_VideoSource )
    m_VideoSource->RemoveObserver( m_VideoSourceObserverTag );

  m_VideoSource = videoSource;

  if( m_VideoSource )
  {
    itk::MemberCommand<QmitkVideoBackground>::Pointer _ModifiedCommand =
      itk::MemberCommand<QmitkVideoBackground>::New();
    _ModifiedCommand->SetCallbackFunction(this, &QmitkVideoBackground::OnVideoSourceDelete);
    m_VideoSourceObserverTag
      = m_VideoSource->AddObserver(itk::DeleteEvent(), _ModifiedCommand);
  }

}

void QmitkVideoBackground::SetTimerDelay( int ms )
{
  m_QTimer->setInterval( ms );
}

mitk::VideoSource* QmitkVideoBackground::GetVideoSource()
{
  return m_VideoSource;
}

int QmitkVideoBackground::GetTimerDelay()
{
  return m_QTimer->interval();
}

void QmitkVideoBackground::OnVideoSourceDelete(const itk::Object*,
                                               const itk::EventObject&)
{
  this->Disable(); // will only disable if enabled
  m_VideoSource = 0;
}

void QmitkVideoBackground::OnRenderWindowDelete( vtkObject * object,
                                                 unsigned long,
                                                 void* clientdata,
                                                 void*)
{
  QmitkVideoBackground* instance = static_cast<QmitkVideoBackground*>( clientdata );
  instance->RemoveRenderWindow( static_cast<vtkRenderWindow*>(object), false );
}

