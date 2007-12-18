/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
  m_Renderer = vtkRenderer::New();
  m_Actor = vtkActor::New();
  m_Mapper = vtkPolyDataMapper::New();
  m_Lut = vtkLookupTable::New();
  m_Plane = vtkPolyData::New();

  vtkPoints* points = vtkPoints::New( ); 
  points->InsertPoint(0,-10,0,0);
  points->InsertPoint(1,-10,1,0);
  points->InsertPoint(2,10,1,0);
  points->InsertPoint(3,10,0,0);

  vtkCellArray* cellArray = vtkCellArray::New();
  cellArray->InsertNextCell(4);
  cellArray->InsertCellPoint(0);
  cellArray->InsertCellPoint(1);
  cellArray->InsertCellPoint(2);
  cellArray->InsertCellPoint(3);

  vtkUnsignedIntArray* data = vtkUnsignedIntArray::New();  
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

  m_Mapper->SetInput( m_Plane );
  m_Mapper->SetLookupTable( m_Lut );

  m_Actor->SetMapper( m_Mapper );

  m_Renderer->AddActor( m_Actor );
  m_Renderer->InteractiveOff();
  m_Renderer->GetActiveCamera()->ParallelProjectionOn();
  m_Renderer->ResetCamera();
  m_Renderer->GetActiveCamera()->SetParallelScale(0.5);
}

mitk::GradientBackground::~GradientBackground()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  
  if ( m_Plane != NULL )
    m_Plane->Delete();
  
  if( m_Lut != NULL )
    m_Lut->Delete();

  if ( m_Mapper != NULL )
    m_Mapper->Delete();
  
  if ( m_Actor!=NULL )
    m_Actor->Delete();
  
  if ( m_Renderer != NULL )
    m_Renderer->Delete();
}

/**
 * Sets the renderwindow, in which the gradient background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::GradientBackground::SetRenderWindow( vtkRenderWindow * renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the gradient background
 */
vtkRenderWindow* mitk::GradientBackground::GetRenderWindow()
{
  return m_RenderWindow;
}

/**
 * Returns the renderer responsible for
 * rendering the color gradient into the
 * vtkRenderWindow
 */
vtkRenderer* mitk::GradientBackground::GetVtkRenderer()
{
  return m_Renderer;
}

/**
 * Returns the actor associated with the color gradient
 */
vtkActor* mitk::GradientBackground::GetActor()
{
  return m_Actor;
}

/**
 * Returns the mapper associated with the color
 * gradient.
 */
vtkPolyDataMapper* mitk::GradientBackground::GetMapper()
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

void mitk::GradientBackground::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::GradientBackground::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;    
}

bool mitk::GradientBackground::VerifyRequestedRegion()
{
    return true;
}

void mitk::GradientBackground::SetRequestedRegion(itk::DataObject*)
{
    //nothing to do
}

