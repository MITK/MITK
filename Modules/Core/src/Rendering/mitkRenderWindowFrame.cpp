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
#include "vtkMitkRectangleProp.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

mitk::RenderWindowFrame::RenderWindowFrame()
{
  m_RenderWindow = nullptr;
  m_RectangleRenderer = vtkSmartPointer<vtkRenderer>::New();

  m_IsEnabled = false;
}

mitk::RenderWindowFrame::~RenderWindowFrame()
{
  if (m_RenderWindow != nullptr)
    if (this->IsEnabled())
      this->Disable();
}

/**
 * Sets the renderwindow, in which the text
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::RenderWindowFrame::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow)
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the text
 */
vtkSmartPointer<vtkRenderWindow> mitk::RenderWindowFrame::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  text into the
 * vtkRenderWindow
 */
vtkSmartPointer<vtkRenderer> mitk::RenderWindowFrame::GetVtkRenderer()
{
  return m_RectangleRenderer;
}

/**
 * Disables drawing of the text label collection.
 * If you want to enable it, call the Enable() function.
 */
void mitk::RenderWindowFrame::Disable()
{
  if (this->IsEnabled())
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
  vtkSmartPointer<vtkMitkRectangleProp> rect = vtkSmartPointer<vtkMitkRectangleProp>::New();
  rect->SetColor(col1, col2, col3);

  m_RectangleRenderer->AddActor(rect);
  if (!mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted(m_RectangleRenderer))
  {
    m_RectangleRenderer->EraseOff();
    m_RectangleRenderer->SetInteractive(0);

    mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertForegroundRenderer(m_RectangleRenderer, true);

    m_IsEnabled = true;
  }
}
/**
 * Checks, if the text is currently
 * enabled (visible)
 */
bool mitk::RenderWindowFrame::IsEnabled()
{
  return m_IsEnabled;
}
