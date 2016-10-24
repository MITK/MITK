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

#include "mitkGradientBackground.h"
#include "mitkVtkLayerController.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

mitk::GradientBackground::GradientBackground()
{
  m_RenderWindow = nullptr;
  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_Renderer->InteractiveOff();
}

mitk::GradientBackground::~GradientBackground()
{
  if (m_RenderWindow != nullptr)
    if (this->IsEnabled())
      this->Disable();
}

/**
 * Sets the renderwindow, in which the gradient background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::GradientBackground::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow)
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the gradient background
 */
vtkSmartPointer<vtkRenderWindow> mitk::GradientBackground::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the color gradient into the
 * vtkRenderWindow
 */
vtkSmartPointer<vtkRenderer> mitk::GradientBackground::GetVtkRenderer()
{
  return m_Renderer;
}

/**
 * Sets the gradient colors. The gradient
 * will smoothly fade from color1 to color2
 */
void mitk::GradientBackground::SetGradientColors(double r1, double g1, double b1, double r2, double g2, double b2)
{
  this->SetLowerColor(r1, g1, b1);
  this->SetUpperColor(r2, g2, b2);
}

void mitk::GradientBackground::SetGradientColors(mitk::Color upper, mitk::Color lower)
{
  this->SetGradientColors(upper[0], upper[1], upper[2], lower[0], lower[1], lower[2]);
}

void mitk::GradientBackground::SetUpperColor(double r, double g, double b)
{
  m_Renderer->SetBackground(r, g, b);
}

void mitk::GradientBackground::SetLowerColor(double r, double g, double b)
{
  m_Renderer->SetBackground2(r, g, b);
}

void mitk::GradientBackground::SetUpperColor(mitk::Color upper)
{
  this->SetUpperColor(upper[0], upper[1], upper[2]);
}

void mitk::GradientBackground::SetLowerColor(mitk::Color lower)
{
  this->SetLowerColor(lower[0], lower[1], lower[2]);
}

/**
 * Enables drawing of the color gradient background.
 * If you want to disable it, call the Disable() function.
 */
void mitk::GradientBackground::Enable()
{
  m_Renderer->GradientBackgroundOn();
  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertBackgroundRenderer(m_Renderer, true);
}

/**
 * Disables drawing of the color gradient background.
 * If you want to enable it, call the Enable() function.
 */
void mitk::GradientBackground::Disable()
{
  if (this->IsEnabled())
  {
    m_Renderer->GradientBackgroundOff();
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_Renderer);
  }
}

/**
 * Checks, if the gradient background is currently
 * enabled (visible)
 */
bool mitk::GradientBackground::IsEnabled()
{
  if (m_RenderWindow == nullptr)
    return false;
  else
    return (mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted(m_Renderer));
}
