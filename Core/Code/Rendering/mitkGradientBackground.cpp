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

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkMapper.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>
#include <vtkLookupTable.h>
#include <vtkCellArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>

mitk::GradientBackground::GradientBackground()
{
  m_RenderWindow = NULL;
  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Lut = vtkSmartPointer<vtkLookupTable>::New();
  m_Plane = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertPoint(0,-10,0,0);
  points->InsertPoint(1,-10,1,0);
  points->InsertPoint(2,10,1,0);
  points->InsertPoint(3,10,0,0);

  vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(0);
  cellArray->InsertCellPoint(1);
  cellArray->InsertCellPoint(2);
  cellArray->InsertCellPoint(3);

  vtkSmartPointer<vtkUnsignedIntArray> data = vtkSmartPointer<vtkUnsignedIntArray>::New();
  data->InsertTuple1(0,1);
  data->InsertTuple1(1,0);
  data->InsertTuple1(2,0);
  data->InsertTuple1(3,1);

  m_Plane->SetPoints( points );
  m_Plane->SetPolys( cellArray );
  m_Plane->GetPointData()->SetScalars( data );

  m_Lut->SetNumberOfColors( 2 );
  m_Lut->Build();
  m_Lut->SetTableValue( m_Lut->GetIndex(0), 1, 1, 1 );
  m_Lut->SetTableValue( m_Lut->GetIndex(1), 0, 0, 0 );

  m_Mapper->SetInputData( m_Plane );
  m_Mapper->SetLookupTable( m_Lut );

  m_Actor->SetMapper( m_Mapper );

  m_Renderer->AddActor( m_Actor );
  m_Renderer->InteractiveOff();
}

mitk::GradientBackground::~GradientBackground()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
}

/**
 * Sets the renderwindow, in which the gradient background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::GradientBackground::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow )
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
 * Returns the actor associated with the color gradient
 */
vtkSmartPointer<vtkActor> mitk::GradientBackground::GetActor()
{
  return m_Actor;
}

/**
 * Returns the mapper associated with the color
 * gradient.
 */
vtkSmartPointer<vtkPolyDataMapper> mitk::GradientBackground::GetMapper()
{
  return m_Mapper;
}

/**
 * Sets the gradient colors. The gradient
 * will smoothly fade from color1 to color2
 */
void mitk::GradientBackground::SetGradientColors( double r1, double g1, double b1, double r2, double g2, double b2 )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(0), r1, g1, b1 );
  m_Lut->SetTableValue( m_Lut->GetIndex(1), r2, g2, b2 );
}

void mitk::GradientBackground::SetUpperColor(double r, double g, double b )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(0), r, g, b );
}

void mitk::GradientBackground::SetLowerColor(double r, double g, double b )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(1), r, g, b );
}

/**
 * Enables drawing of the color gradient background.
 * If you want to disable it, call the Disable() function.
 */
void mitk::GradientBackground::Enable()
{
  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertBackgroundRenderer(m_Renderer,true);
}

/**
 * Disables drawing of the color gradient background.
 * If you want to enable it, call the Enable() function.
 */
void mitk::GradientBackground::Disable()
{
  if ( this->IsEnabled() )
  {
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_Renderer);
  }
}

/**
 * Checks, if the gradient background is currently
 * enabled (visible)
 */
bool mitk::GradientBackground::IsEnabled()
{
    if ( m_RenderWindow == NULL )
        return false;
    else
        return ( mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted(m_Renderer));
}
