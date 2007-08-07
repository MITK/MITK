#include "mitkSimpleTextRendering.h"

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
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkConfigure.h>
#include <vtkTextActor.h>



mitk::SimpleTextRendering::SimpleTextRendering()
{
  m_RenderWindow      = NULL;
  m_TextRenderer      = vtkRenderer::New();

  m_IsEnabled         = false;
}

mitk::SimpleTextRendering::~SimpleTextRendering()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  
  if ( m_TextRenderer != NULL )
    m_TextRenderer->Delete();

}

/**
 * Sets the renderwindow, in which the text
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::SimpleTextRendering::SetRenderWindow( mitk::RenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the text
 */
mitk::RenderWindow* mitk::SimpleTextRendering::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  text into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::SimpleTextRendering::GetVtkRenderer()
{
  return m_TextRenderer;
}


vtkTextProperty* mitk::SimpleTextRendering::GetVtkTextProperty(int text_id)
{
  return this->m_TextCollection[text_id]->GetTextProperty();
}

/**
 * Disables drawing of the text label collection.
 * If you want to enable it, call the Enable() function.
 */
void mitk::SimpleTextRendering::Disable()
{
  if ( this->IsEnabled())
  {
    #if ( VTK_MAJOR_VERSION >= 5 )
      m_TextRenderer->SetErase(1);
    #endif
    m_RenderWindow->GetVtkRenderWindow()->SetErase(1);
    m_RenderWindow->GetVtkLayerController()->RemoveRenderer(m_TextRenderer);
    m_IsEnabled = false;
  }
}

/**
 * Enables drawing of the text label collection.
 * If you want to disable it, call the Disable() function.
 */
void mitk::SimpleTextRendering::Enable()
{
  if(this->IsEnabled())
    this->Disable();

  if(m_TextRenderer != NULL)
    m_TextRenderer->Delete();

  m_TextRenderer = vtkRenderer::New();

  for (TextMapType::iterator it = m_TextCollection.begin(); it != m_TextCollection.end() ; it++)
  {
    m_TextRenderer->AddActor2D((*it).second);
  }
    
  #if ( VTK_MAJOR_VERSION >= 5 )
    m_TextRenderer->SetErase(0);
  #endif
  m_TextRenderer->SetInteractive(0);

  m_RenderWindow->GetVtkRenderWindow()->SetErase(0);  
  m_RenderWindow->GetVtkLayerController()->InsertForegroundRenderer(m_TextRenderer,false);
  
  m_IsEnabled = true;
}
/**
 * Checks, if the text is currently
 * enabled (visible)
 */
bool mitk::SimpleTextRendering::IsEnabled()
{
  return  m_IsEnabled;
}

int mitk::SimpleTextRendering::GetNumberOfTextLabels()
{
  return m_TextCollection.size();
}

void mitk::SimpleTextRendering::ClearTextLabelCollection()
{
  m_TextCollection.clear();
}

int mitk::SimpleTextRendering::AddTextLabel(int posX, int posY, std::string text)
{
  if(text.size() > 0)
  {
    vtkTextActor* textActor = vtkTextActor::New();

    textActor->SetPosition(posX,posY);
    textActor->SetInput(text.c_str());
    
    int text_id = m_TextCollection.size();
    m_TextCollection.insert(TextMapType::value_type(text_id,textActor));
    return text_id;
    
  }
  return -1;
}
void mitk::SimpleTextRendering::RemoveTextLabel(int text_id)
{
  m_TextCollection.erase(text_id);
}
void mitk::SimpleTextRendering::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::SimpleTextRendering::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;    
}

bool mitk::SimpleTextRendering::VerifyRequestedRegion()
{
    return true;
}

void mitk::SimpleTextRendering::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}

