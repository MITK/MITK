#include "mitkGradientBackground.h"
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
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
void mitk::GradientBackground::SetVtkRenderWindow( vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;
}

/**
 * Returns the vtkRenderWindow, which is used
 * for displaying the gradient background
 */
vtkRenderWindow* mitk::GradientBackground::GetVtkRenderWindow()
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
void mitk::GradientBackground::SetGradientColors( vtkFloatingPointType r1, vtkFloatingPointType g1, vtkFloatingPointType b1, vtkFloatingPointType r2, vtkFloatingPointType g2, vtkFloatingPointType b2 )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(0), r1, g1, b1 );
  m_Lut->SetTableValue( m_Lut->GetIndex(1), r2, g2, b2 );
}


void mitk::GradientBackground::SetUpperColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(0), r, g, b );
}


void mitk::GradientBackground::SetLowerColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b )
{
  m_Lut->SetTableValue( m_Lut->GetIndex(1), r, g, b );
}


/**
 * Enables drawing of the color gradient background.
 * If you want to disable it, call the Disable() function.
 */
void mitk::GradientBackground::Enable()
{
  if ( m_RenderWindow == NULL )
    return;
  if ( m_Renderer == NULL )
    return;
  if ( IsEnabled() )
    return;
  
  //
  // layer ordering has changed from vtk 4.4 to vtk 5.0
  //
  #if ( VTK_MAJOR_VERSION >= 5 )
    int foregroundLayer = 1;
    int backgroundLayer = 0;
  #else
    int foregroundLayer = 0;
    int backgroundLayer = 1;
  #endif
  
  //
  // traverse the set of renderers and set their 
  // layer to foreground / background
  //
  m_RenderWindow->SetNumberOfLayers( 2 );
  m_Renderer->SetLayer( backgroundLayer );
  m_RenderWindow->AddRenderer( m_Renderer );
  for( int i = 0 ; i < m_RenderWindow->GetRenderers()->GetNumberOfItems() ; ++i )
  {
    vtkRenderer* currentRenderer =  dynamic_cast<vtkRenderer*>( m_RenderWindow->GetRenderers()->GetItemAsObject(i) );
    if ( currentRenderer != NULL )
    {
      if (currentRenderer == m_Renderer)
        currentRenderer->SetLayer( backgroundLayer );
      else
        currentRenderer->SetLayer( foregroundLayer );
    }
  }
}

/**
 * Disables drawing of the color gradient background.
 * If you want to enable it, call the Enable() function.
 */
void mitk::GradientBackground::Disable()
{
  if ( this->IsEnabled() )
    m_RenderWindow->RemoveRenderer( m_Renderer );
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
        return ( m_RenderWindow->GetRenderers()->IsItemPresent( m_Renderer ) != 0 );    
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

