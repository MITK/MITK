/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-18 16:04:12 +0100 (Di, 18 Dez 2007) $
Version:   $Revision: 13252 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkVtkWidgetRendering.h"

#include "mitkVtkLayerController.h"

#include <mitkConfig.h>
#include <itkObject.h>
#include <itkMacro.h>
#include <itksys/SystemTools.hxx>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>

#include <vtkInteractorObserver.h>

#include <algorithm>


mitk::VtkWidgetRendering::VtkWidgetRendering()
: m_RenderWindow( NULL ),
  m_VtkWidget( NULL ),
  m_IsEnabled( false )
{
  m_Renderer = vtkRenderer::New();
}
 

mitk::VtkWidgetRendering::~VtkWidgetRendering()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  
  if ( m_Renderer != NULL )
    m_Renderer->Delete();
}

/**
 * Sets the renderwindow, in which the widget
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::VtkWidgetRendering::SetRenderWindow( vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the widget
 */
vtkRenderWindow* mitk::VtkWidgetRendering::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the  widget into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::VtkWidgetRendering::GetVtkRenderer()
{
  return m_Renderer;
}

/**
 * Enables drawing of the widget.
 * If you want to disable it, call the Disable() function.
 */
void mitk::VtkWidgetRendering::Enable()
{
  if(m_IsEnabled)
    return;

  if(m_RenderWindow != NULL)
  {
    vtkRenderWindowInteractor *interactor = m_RenderWindow->GetInteractor();

    if ( m_VtkWidget != NULL )
    {
      m_VtkWidget->SetInteractor( interactor );

      mitk::VtkLayerController::GetInstance(m_RenderWindow)
        ->InsertForegroundRenderer(m_Renderer,false);
      
      m_IsEnabled = true;
    }
  }
}


/**
 * Disables drawing of the widget.
 * If you want to enable it, call the Enable() function.
 */
void mitk::VtkWidgetRendering::Disable()
{
  if ( this->IsEnabled() )
  {
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_Renderer);
    m_IsEnabled = false;
  }
}

/**
 * Checks, if the widget is currently
 * enabled (visible)
 */
bool mitk::VtkWidgetRendering::IsEnabled()
{
  return  m_IsEnabled;
}


void mitk::VtkWidgetRendering::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::VtkWidgetRendering::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;    
}

bool mitk::VtkWidgetRendering::VerifyRequestedRegion()
{
    return true;
}

void mitk::VtkWidgetRendering::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}

void mitk::VtkWidgetRendering::SetVtkWidget( vtkInteractorObserver *widget )
{
  m_VtkWidget = widget;
}
