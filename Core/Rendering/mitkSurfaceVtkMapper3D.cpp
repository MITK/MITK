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


#include "mitkSurfaceVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkMaterialProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>

//##ModelId=3E70F60301D5
const mitk::Surface* mitk::SurfaceVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Surface * > ( GetData() );
}

//##ModelId=3E70F60301F4
mitk::SurfaceVtkMapper3D::SurfaceVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_VtkPolyDataNormals = vtkPolyDataNormals::New();

  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);

  m_Prop3D = m_Actor;

  m_GenerateNormals = false;
}

//##ModelId=3E70F60301F5
mitk::SurfaceVtkMapper3D::~SurfaceVtkMapper3D()
{
  m_VtkPolyDataMapper->Delete();
  m_VtkPolyDataNormals->Delete();

  if(m_Prop3D != m_Actor)
    m_Actor->Delete();
}

//##ModelId=3EF19FA803BF
void mitk::SurfaceVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  bool visible = IsVisible(renderer);

  if(visible==false)
  {
    m_Actor->VisibilityOff();
    return;
  }

  //
  // get the TimeSlicedGeometry of the input object
  //
  mitk::Surface::Pointer input  = const_cast< mitk::Surface* >( this->GetInput() );
  const TimeSlicedGeometry* inputTimeGeometry = input->GetTimeSlicedGeometry();
  if(( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ) )
  {
    m_Actor->VisibilityOff();
    return;
  }

  //
  // get the world time
  //
  const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();
  assert( worldGeometry != NULL );
  ScalarType time = worldGeometry->GetTimeBounds()[ 0 ];

  //
  // convert the world time in time steps of the input object
  //
  int timestep=0;
  if( time > ScalarTypeNumericTraits::NonpositiveMin() )
    timestep = inputTimeGeometry->MSToTimeStep( time );
  if( inputTimeGeometry->IsValidTime( timestep ) == false )
  {
    m_Actor->VisibilityOff();
    return;
  }
//  std::cout << "time: "<< time << std::endl;
//  std::cout << "timestep: "<<timestep << std::endl;
  
  //
  // set the input-object at time t for the mapper
  //

  vtkPolyData * polydata = input->GetVtkPolyData( timestep );
  if(polydata == NULL) 
  {
    m_Actor->VisibilityOff();
    return;
  }

  if ( m_GenerateNormals )
  {
    m_VtkPolyDataNormals->SetInput( polydata );
    m_VtkPolyDataMapper->SetInput( m_VtkPolyDataNormals->GetOutput() );
  }
  else
  {
    m_VtkPolyDataMapper->SetInput( polydata );
  }

  //
  // apply properties read from the PropertyList
  //
  ApplyProperties(m_Actor, renderer);

  if(visible)
    m_Actor->VisibilityOn();

  m_Prop3D->SetUserTransform(GetDataTreeNode()->GetVtkTransform(timestep));
}

void mitk::SurfaceVtkMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* renderer)
{
  mitk::MaterialProperty* materialProperty;
  //first check render specific property, then the regulat one
  bool setMaterial = false;
  
  this->GetDataTreeNode()->GetProperty(materialProperty, "material", renderer);
  if ( materialProperty != NULL && this->GetDataTreeNode()->GetPropertyList(renderer)->IsEnabled("material"))
    setMaterial = true;
  else 
  {
    this->GetDataTreeNode()->GetProperty(materialProperty, "material");//without renderer?
    if (materialProperty != NULL && this->GetDataTreeNode()->GetPropertyList()->IsEnabled("material"))
      setMaterial = true;
  }
    
  if (setMaterial)
  {
    vtkProperty* property = m_Actor->GetProperty();
    //property->SetColor( materialProperty->GetColor().GetDataPointer() );
    property->SetAmbientColor( materialProperty->GetColor().GetDataPointer() );    
    property->SetAmbient( materialProperty->GetColorCoefficient() );    
    property->SetDiffuseColor(materialProperty->GetColor().GetDataPointer() );    
    property->SetDiffuse( materialProperty->GetColorCoefficient() );
    property->SetSpecularColor( materialProperty->GetSpecularColor().GetDataPointer() );
    property->SetSpecular( materialProperty->GetSpecularCoefficient() );
    property->SetSpecularPower( materialProperty->GetSpecularPower() );
    property->SetOpacity( materialProperty->GetOpacity() );
    property->SetInterpolation( materialProperty->GetVtkInterpolation() );
    property->SetRepresentation( materialProperty->GetVtkRepresentation() );
    property->SetLineWidth( materialProperty->GetLineWidth() );
  }
  else
  {
    Superclass::ApplyProperties( m_Actor, renderer ) ;
    //reset the default values in case no material is used
    vtkProperty* property = m_Actor->GetProperty();

    property->SetAmbient( 0.0f );    
    property->SetDiffuse( 1.0f );
    property->SetSpecular( 0.0f );
    property->SetSpecularPower( 1.0f );

    int lineWidth = 0;
    this->GetDataTreeNode()->GetIntProperty("wireframe line width", lineWidth);
    m_Actor->GetProperty()->SetLineWidth( lineWidth );
  }

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataTreeNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if (lookupTableProp.IsNotNull() )
	{
    m_VtkPolyDataMapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  mitk::LevelWindow levelWindow;
  if(this->GetDataTreeNode()->GetLevelWindow(levelWindow, renderer, "levelWindow"))
  {
    m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetMin(),levelWindow.GetMax());
  }
  else
  if(this->GetDataTreeNode()->GetLevelWindow(levelWindow, renderer))
  {
    m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetMin(),levelWindow.GetMax());
  }
  
  mitk::VtkRepresentationProperty* representationProperty;
  this->GetDataTreeNode()->GetProperty(representationProperty, "representation", renderer);
  if ( representationProperty != NULL )
    m_Actor->GetProperty()->SetRepresentation( representationProperty->GetVtkRepresentation() );
  
  mitk::VtkInterpolationProperty* interpolationProperty;
  this->GetDataTreeNode()->GetProperty(interpolationProperty, "interpolation", renderer);
  if ( interpolationProperty != NULL )
    m_Actor->GetProperty()->SetInterpolation( interpolationProperty->GetVtkInterpolation() );
  
  bool scalarVisibility = false;
  this->GetDataTreeNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  m_VtkPolyDataMapper->SetScalarVisibility( (scalarVisibility ? 1 : 0) );

  if(scalarVisibility)
  {
    mitk::VtkScalarModeProperty* scalarMode;
    if(this->GetDataTreeNode()->GetProperty(scalarMode, "scalar mode", renderer))
    {
      m_VtkPolyDataMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
    }
    else
      m_VtkPolyDataMapper->SetScalarModeToDefault();

    bool colorMode = false;
    this->GetDataTreeNode()->GetBoolProperty("color mode", colorMode);
    m_VtkPolyDataMapper->SetColorMode( (colorMode ? 1 : 0) );

    float scalarsMin = 0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMinimum")) != NULL)
      scalarsMin = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMinimum"))->GetValue();

    float scalarsMax = 1.0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMaximum")) != NULL)
      scalarsMax = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMaximum"))->GetValue();

    m_VtkPolyDataMapper->SetScalarRange(scalarsMin,scalarsMax);
  }

  // deprecated settings
  bool deprecatedUseCellData = false;
  this->GetDataTreeNode()->GetBoolProperty("deprecated useCellDataForColouring", deprecatedUseCellData);

  bool deprecatedUsePointData = false;
  this->GetDataTreeNode()->GetBoolProperty("deprecated usePointDataForColouring", deprecatedUsePointData);

  if (deprecatedUseCellData)
  {
    m_VtkPolyDataMapper->SetColorModeToDefault();
    m_VtkPolyDataMapper->SetScalarRange(0,255);
    m_VtkPolyDataMapper->ScalarVisibilityOn();
    m_VtkPolyDataMapper->SetScalarModeToUseCellData();
    m_Actor->GetProperty()->SetSpecular (1);
    m_Actor->GetProperty()->SetSpecularPower (50);
    m_Actor->GetProperty()->SetInterpolationToPhong();
  }
  else if (deprecatedUsePointData)
  {
    float scalarsMin = 0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMinimum")) != NULL)
      scalarsMin = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMinimum"))->GetValue();

    float scalarsMax = 0.1;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMaximum")) != NULL)
      scalarsMax = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("ScalarsRangeMaximum"))->GetValue();

    m_VtkPolyDataMapper->SetScalarRange(scalarsMin,scalarsMax);
    m_VtkPolyDataMapper->SetColorModeToMapScalars();
    m_VtkPolyDataMapper->ScalarVisibilityOn();
    m_Actor->GetProperty()->SetSpecular (1);
    m_Actor->GetProperty()->SetSpecularPower (50);
    m_Actor->GetProperty()->SetInterpolationToPhong();
  }

  int deprecatedScalarMode = VTK_COLOR_MODE_DEFAULT;
  if(this->GetDataTreeNode()->GetIntProperty("deprecated scalar mode", deprecatedScalarMode, renderer))
  {
    m_VtkPolyDataMapper->SetScalarMode(deprecatedScalarMode);
    m_VtkPolyDataMapper->ScalarVisibilityOn();
    m_Actor->GetProperty()->SetSpecular (1);
    m_Actor->GetProperty()->SetSpecularPower (50);
    //m_Actor->GetProperty()->SetInterpolationToPhong();
  }
}
