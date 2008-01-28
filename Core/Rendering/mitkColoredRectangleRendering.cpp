#include "mitkColoredRectangleRendering.h"

#include "mitkVtkLayerController.h"

#include <mitkStandardFileLocations.h>
#include <mitkConfig.h>
#include <itkObject.h>
#include <itkMacro.h>
#include <itksys/SystemTools.hxx>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkConfigure.h>

#include <vtkMitkRectangleProp.h>

mitk::ColoredRectangleRendering::ColoredRectangleRendering()
{
  m_RenderWindow           = NULL;
  m_RectangleRenderer      = vtkRenderer::New();

  m_IsEnabled         = false;
}

mitk::ColoredRectangleRendering::~ColoredRectangleRendering()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  
  if ( m_RectangleRenderer != NULL )
    m_RectangleRenderer->Delete();

}

/**
 * Sets the renderwindow, in which the text
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::ColoredRectangleRendering::SetRenderWindow( vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the text
 */
vtkRenderWindow* mitk::ColoredRectangleRendering::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  text into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::ColoredRectangleRendering::GetVtkRenderer()
{
  return m_RectangleRenderer;
}

/**
 * Disables drawing of the text label collection.
 * If you want to enable it, call the Enable() function.
 */
void mitk::ColoredRectangleRendering::Disable()
{
  if ( this->IsEnabled())
  {
    #if ( VTK_MAJOR_VERSION >= 5 )
      m_RectangleRenderer->EraseOn();
    #else
      m_RenderWindow->SetErase(1);
    #endif
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_RectangleRenderer);
    m_IsEnabled = false;
  }
}

/**
 * Enables drawing of the text label collection.
 * If you want to disable it, call the Disable() function.
 */
void mitk::ColoredRectangleRendering::Enable(float col1, float col2, float col3)
{
  vtkMitkRectangleProp* rect = vtkMitkRectangleProp::New();
  rect->SetRenderWindow(m_RenderWindow);
  rect->SetColor(col1, col2, col3);

  m_RectangleRenderer->AddViewProp(rect);
    
  if(!mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted( m_RectangleRenderer ))
  {
    #if ( VTK_MAJOR_VERSION >= 5 )
      m_RectangleRenderer->EraseOff();
    #else
      m_RenderWindow->SetErase(0); 
    #endif
    m_RectangleRenderer->SetInteractive(0);
    
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertForegroundRenderer(m_RectangleRenderer,true);
    
    m_IsEnabled = true;
  }
}
/**
 * Checks, if the text is currently
 * enabled (visible)
 */
bool mitk::ColoredRectangleRendering::IsEnabled()
{
  return  m_IsEnabled;
}
 
void mitk::ColoredRectangleRendering::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::ColoredRectangleRendering::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;    
}

bool mitk::ColoredRectangleRendering::VerifyRequestedRegion()
{
    return true;
}

void mitk::ColoredRectangleRendering::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}

