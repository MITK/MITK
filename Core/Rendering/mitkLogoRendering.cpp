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
#include <vtkImageActor.h>
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
  m_Actor         = vtkImageActor::New();
  m_Mapper        = vtkImageMapper::New();
  m_PngReader     = vtkPNGReader::New();
  
  m_LogoPosition  = mitk::LogoRendering::LowerRight;
 
  m_IsEnabled = false;

  m_ZoomFactor = 1.0;
  m_Opacity    = 1.0;

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
vtkImageActor* mitk::LogoRendering::GetActor()
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

void mitk::LogoRendering::SetLogoSource(const char* filename)
{
  std::string file = filename;
  if(file.length() != 0)
  {
    m_FileName  = filename;
    m_PngReader->SetFileName(m_FileName.c_str());
  }
}

/**
 * Enables drawing of the logo.
 * If you want to disable it, call the Disable() function.
 */
void mitk::LogoRendering::Enable()
{
  if(itksys::SystemTools::FileExists(m_FileName.c_str()) && m_RenderWindow != NULL)
  {
    m_PngReader->Update();
    m_Actor->SetInput(m_PngReader->GetOutput());
  
    m_Actor->SetOpacity(0.85);//m_Opacity);
    m_Renderer->AddActor( m_Actor );
    
    SetupCamera();
    SetupPosition();

    m_Renderer->InteractiveOff();
    m_RenderWindow->GetVtkLayerController()->InsertForegroundRenderer(m_Renderer,true);
    
    m_IsEnabled = true;
  }
}


void mitk::LogoRendering::SetupCamera()
{
  // set the vtk camera in way that stretches the logo all over the renderwindow

  vtkImageData * image = m_Actor->GetInput();
  m_Camera = m_Renderer->GetActiveCamera();
  m_Camera->SetClippingRange(1,100000);

  if ( !image )
    return;
  
  vtkFloatingPointType spacing[3];
  vtkFloatingPointType origin[3];
  int   dimensions[3];

  image->GetSpacing(spacing);
  image->GetOrigin(origin);
  image->GetDimensions(dimensions);

  double focalPoint[3];
  double position[3];

  for ( unsigned int cc = 0; cc < 3; cc++)
  {
    focalPoint[cc] = origin[cc] + ( spacing[cc] * dimensions[cc] ) / 2.0;
    position[cc]   = focalPoint[cc];
  }


  m_Camera->SetViewUp (0,1,0);
  int idx = 2;
  const double distanceToFocalPoint = 1000;
  position[idx] = distanceToFocalPoint;

  m_Camera->ParallelProjectionOn();
  m_Camera->SetPosition (position);
  m_Camera->SetFocalPoint (focalPoint);

  #define myMAX(x,y) (((x)>(y))?(x):(y))  

  int d1 = (idx + 1) % 3;
  int d2 = (idx + 2) % 3;

  double max = myMAX( 
    spacing[d1] * dimensions[d1],
    spacing[d2] * dimensions[d2]);

  m_Camera->SetParallelScale( max );
}

void mitk::LogoRendering::SetupPosition()
{ // Position and Scale of the logo
  
  vtkFloatingPointType newPos[4];
  int dimensions[3];
  vtkImageData * image = m_Actor->GetInput();
  image->GetDimensions(dimensions);
  // normalize image dimensions
  #define myMAX(x,y) (((x)>(y))?(x):(y)) 
  double max = myMAX(dimensions[0],dimensions[1]);
  double normX = dimensions[0] / max;
  double normY = dimensions[1] / max;

  double buffer = 0.02; // buffer to the boarder of the renderwindow
 
  switch(m_LogoPosition)
  {
    case mitk::LogoRendering::LowerLeft:
    {
      newPos[0] = (0 + buffer);
      newPos[1] = (0 + buffer);
      newPos[2] = 0.2 * normX * m_ZoomFactor;
      newPos[3] = 0.2 * normY * m_ZoomFactor;
      break;
    }
    case mitk::LogoRendering::LowerRight:
    {
      newPos[0] = (1 - buffer) - 0.2 * normX * m_ZoomFactor;
      newPos[1] = 0.0;
      newPos[2] = (1 - buffer);
      newPos[3] = 0.2 * normY * m_ZoomFactor;
      break;
    }
    case mitk::LogoRendering::UpperLeft:
    {
      newPos[0] = (0 + buffer);
      newPos[1] = (1 - buffer) - 0.2 * normY * m_ZoomFactor;
      newPos[2] = 0.2 * normX * m_ZoomFactor;
      newPos[3] = (1 - buffer);
      break;
    }
    case mitk::LogoRendering::UpperRight:
    {
      newPos[0] = (1 - buffer) - 0.2 * normX * m_ZoomFactor;
      newPos[1] = (1 - buffer) - 0.2 * normY * m_ZoomFactor;
      newPos[2] = (1 - buffer);
      newPos[3] = (1 - buffer);
      break;
    }
  }
    
  m_Renderer->SetViewport(newPos);
}

void mitk::LogoRendering::SetZoomFactor( double factor )
{
  m_ZoomFactor = factor;
}
void mitk::LogoRendering::SetOpacity(double opacity)
{
  m_Opacity = opacity;
}

/**
 * Disables drawing of the logo.
 * If you want to enable it, call the Enable() function.
 */
void mitk::LogoRendering::Disable()
{
  if ( this->IsEnabled() )
  {
    m_RenderWindow->GetVtkLayerController()->RemoveRenderer(m_Renderer);
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

