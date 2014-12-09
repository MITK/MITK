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

#include "mitkSurfaceVtkMapper2D.h"

//mitk includes
#include <mitkDataNode.h>
#include <mitkProperties.h>
#include "mitkVtkPropRenderer.h"
#include <mitkSurface.h>
#include <mitkLookupTableProperty.h>
#include <mitkVtkScalarModeProperty.h>

//vtk includes
#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPropAssembly.h>
#include <vtkPointData.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// constructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Mapper->ScalarVisibilityOff();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Actor->SetMapper( m_Mapper );
  m_PropAssembly = vtkSmartPointer <vtkPropAssembly>::New();
  m_PropAssembly->AddPart( m_Actor );
  m_CuttingPlane = vtkSmartPointer<vtkPlane>::New();
  m_Cutter = vtkSmartPointer<vtkCutter>::New();
  m_Cutter->SetCutFunction(m_CuttingPlane);
  m_Mapper->SetInputConnection( m_Cutter->GetOutputPort() );
}

// destructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::~LocalStorage()
{
}

const mitk::Surface* mitk::SurfaceVtkMapper2D::GetInput() const
{
  return static_cast<const mitk::Surface * > ( GetDataNode()->GetData() );
}

// constructor PointSetVtkMapper2D
mitk::SurfaceVtkMapper2D::SurfaceVtkMapper2D()
  : m_DrawNormals(false),
    m_FrontNormalLengthInPixels(10.0),
    m_BackNormalLengthInPixels(10.0)
  {
    // default for normals on front side = green
    m_FrontSideColor[0] = 0.0;
    m_FrontSideColor[1] = 1.0;
    m_FrontSideColor[2] = 0.0;
    m_FrontSideColor[3] = 1.0;

    // default for normals on back side = red
    m_BackSideColor[0] = 1.0;
    m_BackSideColor[1] = 0.0;
    m_BackSideColor[2] = 0.0;
    m_BackSideColor[3] = 1.0;

    // default for line color = yellow
    m_LineColor[0] = 1.0;
    m_LineColor[1] = 1.0;
    m_LineColor[2] = 0.0;
    m_LineColor[3] = 1.0;
}

mitk::SurfaceVtkMapper2D::~SurfaceVtkMapper2D()
{
}

// reset mapper so that nothing is displayed e.g. toggle visiblity of the propassembly
void mitk::SurfaceVtkMapper2D::ResetMapper( BaseRenderer* renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_PropAssembly->VisibilityOff();
}

vtkProp* mitk::SurfaceVtkMapper2D::GetVtkProp(mitk::BaseRenderer * renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_PropAssembly;
}

void mitk::SurfaceVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  const mitk::DataNode* node = GetDataNode();
  if( node == NULL )
    return;
  bool visible = true;

  node->GetVisibility(visible, renderer, "visible");

  if ( !visible )
  {
    return;
  }

  mitk::Surface* surface  = static_cast<mitk::Surface *>( node->GetData() );
  if ( surface == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const mitk::TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();
  if ( ( dataTimeGeometry == NULL )
       || ( dataTimeGeometry->CountTimeSteps() == 0 )
       || ( !dataTimeGeometry->IsValidTimeStep( this->GetTimestep() ) ) )
  {
    return;
  }

  surface->UpdateOutputInformation();
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
       || (localStorage->m_LastUpdateTime < surface->GetPipelineMTime()) //Was the data modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) //was the geometry modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime())
       || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
       || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::SurfaceVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  const mitk::DataNode* node = GetDataNode();
  mitk::Surface* surface  = static_cast<mitk::Surface *>( node->GetData() );
  const mitk::TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  ScalarType time =renderer->GetTime();
  int timestep=0;

  if( time > itk::NumericTraits<mitk::ScalarType>::NonpositiveMin() )
    timestep = dataTimeGeometry->TimePointToTimeStep( time );

  vtkSmartPointer<vtkPolyData> inputPolyData = surface->GetVtkPolyData( timestep );
  if((inputPolyData==NULL) || (inputPolyData->GetNumberOfPoints() < 1 ))
    return;

  //apply color and opacity read from the PropertyList
  this->ApplyAllProperties(renderer);

  const PlaneGeometry* planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if( ( planeGeometry == NULL ) || ( !planeGeometry->IsValid() ) || ( !planeGeometry->HasReferenceGeometry() ))
  {
    return;
  }

  double origin[3];
  origin[0] = planeGeometry->GetOrigin()[0];
  origin[1] = planeGeometry->GetOrigin()[1];
  origin[2] = planeGeometry->GetOrigin()[2];

  double normal[3];
  normal[0] = planeGeometry->GetNormal()[0];
  normal[1] = planeGeometry->GetNormal()[1];
  normal[2] = planeGeometry->GetNormal()[2];

  localStorage->m_CuttingPlane->SetOrigin(origin);
  localStorage->m_CuttingPlane->SetNormal(normal);

  localStorage->m_Cutter->SetInputData(inputPolyData);
  localStorage->m_Cutter->Update();

  //By default, the cutter will also copy/compute normals of the cut
  //to the output polydata. The normals will be rendered by the
  //vtkPolyDataMapper in a (weird) way. It seems that there is no way
  //to turn off that rendering or to stop the computation. Setting
  //the normals to NULL produces our desired (clean) output.
  localStorage->m_Cutter->GetOutput()->GetPointData()->SetNormals(NULL);

//  vtkSmartPointer<vtkRenderer> vtktmprenderer = vtkSmartPointer<vtkRenderer>::New();
//  vtktmprenderer->AddActor(localStorage->m_Actor); //display the cube

//  //Add renderer to renderwindow and render
//  vtkSmartPointer<vtkRenderWindow> renderWindow =
//    vtkSmartPointer<vtkRenderWindow>::New();
//  renderWindow->AddRenderer(vtktmprenderer);
//  renderWindow->SetSize(600, 600);

//  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//    vtkSmartPointer<vtkRenderWindowInteractor>::New();
//  interactor->SetRenderWindow(renderWindow);
//  renderWindow->Render();

//  interactor->Start();
}

void mitk::SurfaceVtkMapper2D::ApplyAllProperties(mitk::BaseRenderer* renderer)
{
  DataNode * node = GetDataNode();

  if(node == NULL)
  {
    return;
  }

  node->GetBoolProperty("draw normals 2D", m_DrawNormals, renderer);

  float lineWidth = 1.0f;
  node->GetFloatProperty("line width", lineWidth, renderer);

  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  // check for color and opacity properties, use it for rendering if they exists
  float color[3]= { 1.0f, 1.0f, 1.0f };
  node->GetColor(color, renderer, "color");
  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  localStorage->m_Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  localStorage->m_Actor->GetProperty()->SetLineWidth(lineWidth);

  bool invertNormals(false);
  node->GetBoolProperty("invert normals", invertNormals, renderer);

  if (!invertNormals)
  {
    node->GetColor(m_FrontSideColor, renderer, "front color");
    node->GetOpacity(m_FrontSideColor[3], renderer, "opacity");

    node->GetColor(m_BackSideColor, renderer, "back color");
    node->GetOpacity(m_BackSideColor[3], renderer, "opacity");

    node->GetFloatProperty( "front normal lenth (px)", m_FrontNormalLengthInPixels, renderer );
    node->GetFloatProperty( "back normal lenth (px)", m_BackNormalLengthInPixels, renderer );
  }
  else
  {
    node->GetColor(m_FrontSideColor, renderer, "back color");
    node->GetOpacity(m_FrontSideColor[3], renderer, "opacity");

    node->GetColor(m_BackSideColor, renderer, "front color");
    node->GetOpacity(m_BackSideColor[3], renderer, "opacity");

    node->GetFloatProperty( "back normal lenth (px)", m_FrontNormalLengthInPixels, renderer );
    node->GetFloatProperty( "front normal lenth (px)", m_BackNormalLengthInPixels, renderer );
  }
}

void mitk::SurfaceVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", FloatProperty::New(2.0f), renderer, overwrite );
  node->AddProperty( "scalar mode", VtkScalarModeProperty::New(), renderer, overwrite );
  node->AddProperty( "draw normals 2D", BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "invert normals", BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "front color", ColorProperty::New(0.0, 1.0, 0.0), renderer, overwrite );
  node->AddProperty( "back color", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite );
  node->AddProperty( "front normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite );
  node->AddProperty( "back normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite );
  node->AddProperty( "layer", IntProperty::New(100), renderer, overwrite);
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
