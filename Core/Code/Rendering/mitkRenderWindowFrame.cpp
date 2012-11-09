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

#include "mitkRenderWindowFrame.h"

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

mitk::RenderWindowFrame::RenderWindowFrame()
{
  m_RenderWindow           = NULL;
  m_RectangleRenderer      = vtkRenderer::New();

  m_IsEnabled         = false;
}

mitk::RenderWindowFrame::~RenderWindowFrame()
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
void mitk::RenderWindowFrame::SetRenderWindow( vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the text
 */
vtkRenderWindow* mitk::RenderWindowFrame::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  text into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::RenderWindowFrame::GetVtkRenderer()
{
  return m_RectangleRenderer;
}

/**
 * Disables drawing of the text label collection.
 * If you want to enable it, call the Enable() function.
 */
void mitk::RenderWindowFrame::Disable()
{
  if ( this->IsEnabled())
  {
    m_RectangleRenderer->EraseOn();
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_RectangleRenderer);
    m_IsEnabled = false;
  }
}

/**
 * Enables drawing of the text label collection.
 * If you want to disable it, call the Disable() function.
 */
void mitk::RenderWindowFrame::Enable(float col1, float col2, float col3)
{
  vtkMitkRectangleProp* rect = vtkMitkRectangleProp::New();
  rect->SetRenderWindow(m_RenderWindow);
  rect->SetColor(col1, col2, col3);

  m_RectangleRenderer->AddViewProp(rect);

  rect->Delete();

  if(!mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted( m_RectangleRenderer ))
  {

    m_RectangleRenderer->EraseOff();

    m_RectangleRenderer->SetInteractive(0);

    mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertForegroundRenderer(m_RectangleRenderer,true);

    m_IsEnabled = true;
  }
}
/**
 * Checks, if the text is currently
 * enabled (visible)
 */
bool mitk::RenderWindowFrame::IsEnabled()
{
  return  m_IsEnabled;
}

void mitk::RenderWindowFrame::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::RenderWindowFrame::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::RenderWindowFrame::VerifyRequestedRegion()
{
    return true;
}

void mitk::RenderWindowFrame::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}

