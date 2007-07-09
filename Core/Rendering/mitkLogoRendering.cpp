#include "mitkLogoRendering.h"

#include "mitkVtkLayerController.h"
#include <mitkVtkRenderWindow.h>
#include <mitkRenderWindow.h>

#include <mitkStandardFileLocations.h>
#include <mitkConfig.h>
#include <itkObject.h>
#include <itkMacro.h>
#include <itksys/SystemTools.hxx>

#include <vtkRenderer.h>
#include <vtkMapper.h>
#include <vtkActor2D.h>
#include <vtkImageMapper.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkPNGReader.h>
#include <vtkImageData.h>


mitk::LogoRendering::LogoRendering()
{
  m_RenderWindow  = NULL;
  m_Renderer      = vtkRenderer::New();
  m_Actor         = vtkActor2D::New();
  m_Mapper        = vtkImageMapper::New();
  m_PngReader     = vtkPNGReader::New();
  
  m_SizeX         = 200;
  m_SizeY         = 200;
  m_PositionX     = 0;
  m_PositionY     = 0;  

  m_LogoPosition  = mitk::LogoRendering::LogoPosition::LowerRight;
  
  m_IsEnabled = false;

  m_FileName  = mitk::StandardFileLocations::FindFile("mbilogo.png","./mbi-sb/Core/Rendering");
  m_PngReader->SetFileName(m_FileName.c_str());
}

mitk::LogoRendering::~LogoRendering()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  
  if ( m_Mapper != NULL )
    m_Mapper->Delete();
  
  if ( m_Actor!=NULL )
    m_Actor->Delete();
  
  if ( m_Renderer != NULL )
    m_Renderer->Delete();

  if ( m_PngReader != NULL )
    m_PngReader->Delete();
}

/**
 * Sets the renderwindow, in which the logo
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::LogoRendering::SetRenderWindow( mitk::RenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the logo
 */
mitk::RenderWindow* mitk::LogoRendering::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  logo into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::LogoRendering::GetVtkRenderer()
{
  return m_Renderer;
}

/**
 * Returns the actor associated with the  logo
 */
vtkActor2D* mitk::LogoRendering::GetActor()
{
  return m_Actor;
}

/**
 * Returns the mapper associated with the 
 * logo.
 */
vtkImageMapper* mitk::LogoRendering::GetMapper()
{
  return m_Mapper;
}

void mitk::LogoRendering::SetLogoSource(std::string filename)
{
  m_FileName  = filename;
  m_PngReader->SetFileName(m_FileName.c_str());
}

/**
 * Enables drawing of the logo.
 * If you want to disable it, call the Disable() function.
 */
void mitk::LogoRendering::Enable()
{
  if(itksys::SystemTools::FileExists(m_FileName.c_str()))
  {
    m_PngReader->Update();
    m_Mapper->SetInput(m_PngReader->GetOutput());
    
    double range[2];
    m_PngReader->GetOutput()->GetScalarRange(range);
    m_Mapper->SetColorWindow(range[1] - range[0] );
    m_Mapper->SetColorLevel((range[0]+range[1])/2);
    
    // calculate position
    m_Actor->SetPosition(0,0);
    
    
    m_Actor->SetHeight(200);
    m_Actor->SetMapper(m_Mapper);
    m_Actor->SetLayerNumber(99);

    m_Renderer->AddActor( m_Actor );
    m_Renderer->InteractiveOff();
    m_Renderer->GetActiveCamera()->ParallelProjectionOn();
    m_Renderer->ResetCamera();
    m_Renderer->GetActiveCamera()->SetParallelScale(0.5);

    m_RenderWindow->GetVtkLayerController()->GetSceneRenderer()->AddActor2D(m_Actor);

    m_IsEnabled = true;
  }
}

/**
 * Disables drawing of the logo.
 * If you want to enable it, call the Enable() function.
 */
void mitk::LogoRendering::Disable()
{
  if ( this->IsEnabled() )
  {
    m_RenderWindow->GetVtkLayerController()->GetSceneRenderer()->RemoveActor2D(m_Actor);
    m_IsEnabled = false;
  }
}

/**
 * Checks, if the logo is currently
 * enabled (visible)
 */
bool mitk::LogoRendering::IsEnabled()
{
  return  m_IsEnabled;
}


void mitk::LogoRendering::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::LogoRendering::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;    
}

bool mitk::LogoRendering::VerifyRequestedRegion()
{
    return true;
}

void mitk::LogoRendering::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}