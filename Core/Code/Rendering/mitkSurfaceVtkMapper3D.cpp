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
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkClippingProperty.h"

#include "mitkShaderEnumProperty.h"
#include "mitkShaderRepository.h"


#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkPlaneCollection.h>


const mitk::Surface* mitk::SurfaceVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Surface * > ( GetData() );
}

mitk::SurfaceVtkMapper3D::SurfaceVtkMapper3D()
{
 // m_Prop3D = vtkActor::New();
  m_GenerateNormals = false;
}

mitk::SurfaceVtkMapper3D::~SurfaceVtkMapper3D()
{
 // m_Prop3D->Delete();                                  
}

void mitk::SurfaceVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  
  bool visible = IsVisible(renderer);

  if(visible==false)
  {
    ls->m_Actor->VisibilityOff();
    return;
  }

  //
  // set the input-object at time t for the mapper
  //
  mitk::Surface::Pointer input  = const_cast< mitk::Surface* >( this->GetInput() );
  vtkPolyData * polydata = input->GetVtkPolyData( this->GetTimestep() );
  if(polydata == NULL) 
  {
    ls->m_Actor->VisibilityOff();
    return;
  }

  if ( m_GenerateNormals )
  {
    ls->m_VtkPolyDataNormals->SetInput( polydata );
    ls->m_VtkPolyDataMapper->SetInput( ls->m_VtkPolyDataNormals->GetOutput() );
  }
  else
  {
    ls->m_VtkPolyDataMapper->SetInput( polydata );
  }

  //
  // apply properties read from the PropertyList
  //
  ApplyProperties(ls->m_Actor, renderer);

  if(visible)
    ls->m_Actor->VisibilityOn();
}


void mitk::SurfaceVtkMapper3D::ResetMapper( BaseRenderer* renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_Actor->VisibilityOff();
}

void mitk::SurfaceVtkMapper3D::ApplyMitkPropertiesToVtkProperty(mitk::DataNode *node, vtkProperty* property, mitk::BaseRenderer* renderer)
{
  // Colors
  {
    double ambient [3] = { 0.5,0.5,0.0 };
    double diffuse [3] = { 0.5,0.5,0.0 };
    double specular[3] = { 1.0,1.0,1.0 };
    
    float coeff_ambient = 0.5f;
    float coeff_diffuse = 0.5f;
    float coeff_specular= 0.5f;
    float power_specular=10.0f;
  
    // Color
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "color", renderer);
      if(p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        ambient[0]=c.GetRed(); ambient[1]=c.GetGreen(); ambient[2]=c.GetBlue();
        diffuse[0]=c.GetRed(); diffuse[1]=c.GetGreen(); diffuse[2]=c.GetBlue();
        // Setting specular color to the same, make physically no real sense, however vtk rendering slows down, if these colors are different.
        specular[0]=c.GetRed(); specular[1]=c.GetGreen(); specular[2]=c.GetBlue();
      }          
    }
    
    // Ambient 
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.ambientColor", renderer);
      if(p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        ambient[0]=c.GetRed(); ambient[1]=c.GetGreen(); ambient[2]=c.GetBlue();
      }          
    }
    
    // Diffuse 
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.diffuseColor", renderer);
      if(p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        diffuse[0]=c.GetRed(); diffuse[1]=c.GetGreen(); diffuse[2]=c.GetBlue();
      }          
    }
    
    // Specular 
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.specularColor", renderer);
      if(p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        specular[0]=c.GetRed(); specular[1]=c.GetGreen(); specular[2]=c.GetBlue();
      }          
    }

    // Ambient coeff
    {
      node->GetFloatProperty("material.ambientCoefficient", coeff_ambient, renderer);
    }
    
    // Diffuse coeff
    {
      node->GetFloatProperty("material.diffuseCoefficient", coeff_diffuse, renderer);
    }
    
    // Specular coeff
    {
      node->GetFloatProperty("material.specularCoefficient", coeff_specular, renderer);
    }
    
    // Specular power
    {
      node->GetFloatProperty("material.specularPower", power_specular, renderer);
    }
    
    property->SetAmbient( coeff_ambient );    
    property->SetDiffuse( coeff_diffuse );
    property->SetSpecular( coeff_specular );
    property->SetSpecularPower( power_specular );

    property->SetAmbientColor( ambient );    
    property->SetDiffuseColor( diffuse );    
    property->SetSpecularColor( specular );
  }

  // Render mode
  {
    // Opacity
    {
      float opacity = 1.0f;
      if( node->GetOpacity(opacity,renderer) )
        property->SetOpacity( opacity );
    }
     
    // Wireframe line width
    {
      float lineWidth = 1;
      node->GetFloatProperty("material.wireframeLineWidth", lineWidth, renderer);
      property->SetLineWidth( lineWidth );
    }

    // Representation
    {
      mitk::VtkRepresentationProperty::Pointer p;
      node->GetProperty(p, "material.representation", renderer);
      if(p.IsNotNull())
        property->SetRepresentation( p->GetVtkRepresentation() );
    }

    // Interpolation
    {
      mitk::VtkInterpolationProperty::Pointer p;
      node->GetProperty(p, "material.interpolation", renderer);
      if(p.IsNotNull())
        property->SetInterpolation( p->GetVtkInterpolation() );
    }
  }
}



void mitk::SurfaceVtkMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  // Applying shading properties    
  {
    Superclass::ApplyProperties( ls->m_Actor, renderer ) ;
    // VTK Properties
    ApplyMitkPropertiesToVtkProperty( this->GetDataNode(), ls->m_Actor->GetProperty(), renderer );
    // Shaders
    mitk::ShaderRepository::GetGlobalShaderRepository()->ApplyProperties(this->GetDataNode(),ls->m_Actor,renderer,ls->m_ShaderTimestampUpdate);
  }

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if (lookupTableProp.IsNotNull() )
  {
    ls->m_VtkPolyDataMapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  mitk::LevelWindow levelWindow;
  if(this->GetDataNode()->GetLevelWindow(levelWindow, renderer, "levelWindow"))
  {
    ls->m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetLowerWindowBound(),levelWindow.GetUpperWindowBound());
  }
  else
  if(this->GetDataNode()->GetLevelWindow(levelWindow, renderer))
  {
    ls->m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetLowerWindowBound(),levelWindow.GetUpperWindowBound());
  }
  
  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  ls->m_VtkPolyDataMapper->SetScalarVisibility( (scalarVisibility ? 1 : 0) );

  if(scalarVisibility)
  {
    mitk::VtkScalarModeProperty* scalarMode;
    if(this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer))
    {
      ls->m_VtkPolyDataMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
    }
    else
      ls->m_VtkPolyDataMapper->SetScalarModeToDefault();

    bool colorMode = false;
    this->GetDataNode()->GetBoolProperty("color mode", colorMode);
    ls->m_VtkPolyDataMapper->SetColorMode( (colorMode ? 1 : 0) );

    float scalarsMin = 0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum")) != NULL)
      scalarsMin = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum"))->GetValue();

    float scalarsMax = 1.0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum")) != NULL)
      scalarsMax = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum"))->GetValue();

    ls->m_VtkPolyDataMapper->SetScalarRange(scalarsMin,scalarsMax);
  }

  // deprecated settings
  bool deprecatedUseCellData = false;
  this->GetDataNode()->GetBoolProperty("deprecated useCellDataForColouring", deprecatedUseCellData);

  bool deprecatedUsePointData = false;
  this->GetDataNode()->GetBoolProperty("deprecated usePointDataForColouring", deprecatedUsePointData);
                  
  if (deprecatedUseCellData)
  {
    ls->m_VtkPolyDataMapper->SetColorModeToDefault();
    ls->m_VtkPolyDataMapper->SetScalarRange(0,255);
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_VtkPolyDataMapper->SetScalarModeToUseCellData();
    ls->m_Actor->GetProperty()->SetSpecular (1);
    ls->m_Actor->GetProperty()->SetSpecularPower (50);
    ls->m_Actor->GetProperty()->SetInterpolationToPhong();
  }
  else if (deprecatedUsePointData)
  {
    float scalarsMin = 0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum")) != NULL)
      scalarsMin = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum"))->GetValue();

    float scalarsMax = 0.1;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum")) != NULL)
      scalarsMax = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum"))->GetValue();

    ls->m_VtkPolyDataMapper->SetScalarRange(scalarsMin,scalarsMax);
    ls->m_VtkPolyDataMapper->SetColorModeToMapScalars();
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_Actor->GetProperty()->SetSpecular (1);
    ls->m_Actor->GetProperty()->SetSpecularPower (50);
    ls->m_Actor->GetProperty()->SetInterpolationToPhong();
  }

  int deprecatedScalarMode = VTK_COLOR_MODE_DEFAULT;
  if(this->GetDataNode()->GetIntProperty("deprecated scalar mode", deprecatedScalarMode, renderer))
  {
    ls->m_VtkPolyDataMapper->SetScalarMode(deprecatedScalarMode);
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_Actor->GetProperty()->SetSpecular (1);
    ls->m_Actor->GetProperty()->SetSpecularPower (50);
    //m_Actor->GetProperty()->SetInterpolationToPhong();
  }


  // Check whether one or more ClippingProperty objects have been defined for
  // this node. Check both renderer specific and global property lists, since
  // properties in both should be considered.
  const PropertyList::PropertyMap *rendererProperties = this->GetDataNode()->GetPropertyList( renderer )->GetMap();
  const PropertyList::PropertyMap *globalProperties = this->GetDataNode()->GetPropertyList( NULL )->GetMap();

  // Add clipping planes (if any)
  ls->m_ClippingPlaneCollection->RemoveAllItems();
  
  PropertyList::PropertyMap::const_iterator it;
  for ( it = rendererProperties->begin(); it != rendererProperties->end(); ++it )
  {
    this->CheckForClippingProperty( renderer,(*it).second.first.GetPointer() );
  }

  for ( it = globalProperties->begin(); it != globalProperties->end(); ++it )
  {
    this->CheckForClippingProperty( renderer,(*it).second.first.GetPointer() );
  }

  if ( ls->m_ClippingPlaneCollection->GetNumberOfItems() > 0 )
  {
    ls->m_VtkPolyDataMapper->SetClippingPlanes( ls->m_ClippingPlaneCollection );
  }
  else
  {
    ls->m_VtkPolyDataMapper->RemoveAllClippingPlanes();
  }
  
  
}

vtkProp *mitk::SurfaceVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_Actor;
}

void mitk::SurfaceVtkMapper3D::CheckForClippingProperty( mitk::BaseRenderer* renderer, mitk::BaseProperty *property )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
//  m_Prop3D = ls->m_Actor;

  ClippingProperty *clippingProperty = dynamic_cast< ClippingProperty * >( property );

  if ( (clippingProperty != NULL)
    && (clippingProperty->GetClippingEnabled()) )
  {
    const Point3D &origin = clippingProperty->GetOrigin();
    const Vector3D &normal = clippingProperty->GetNormal();

    vtkPlane *clippingPlane = vtkPlane::New();
    clippingPlane->SetOrigin( origin[0], origin[1], origin[2] );
    clippingPlane->SetNormal( normal[0], normal[1], normal[2] );

    ls->m_ClippingPlaneCollection->AddItem( clippingPlane );

    clippingPlane->UnRegister( NULL );
  }
}


void mitk::SurfaceVtkMapper3D::SetDefaultPropertiesForVtkProperty(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  // Shading
  {
    node->AddProperty( "material.wireframeLineWidth", mitk::FloatProperty::New(1.0f)          , renderer, overwrite );

    node->AddProperty( "material.ambientCoefficient" , mitk::FloatProperty::New(0.05f)          , renderer, overwrite );
    node->AddProperty( "material.diffuseCoefficient" , mitk::FloatProperty::New(0.9f)          , renderer, overwrite );
    node->AddProperty( "material.specularCoefficient", mitk::FloatProperty::New(1.0f)          , renderer, overwrite );
    node->AddProperty( "material.specularPower"      , mitk::FloatProperty::New(16.0f)          , renderer, overwrite );

    //node->AddProperty( "material.ambientColor"       , mitk::ColorProperty::New(1.0f,1.0f,1.0f), renderer, overwrite );
    //node->AddProperty( "material.diffuseColor"       , mitk::ColorProperty::New(1.0f,1.0f,1.0f), renderer, overwrite );
    //node->AddProperty( "material.specularColor"      , mitk::ColorProperty::New(1.0f,1.0f,1.0f), renderer, overwrite );

    node->AddProperty( "material.representation"      , mitk::VtkRepresentationProperty::New()  , renderer, overwrite );
    node->AddProperty( "material.interpolation"       , mitk::VtkInterpolationProperty::New()   , renderer, overwrite );
  }
}


void mitk::SurfaceVtkMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "color", mitk::ColorProperty::New(1.0f,1.0f,1.0f), renderer, overwrite );
  node->AddProperty( "opacity", mitk::FloatProperty::New(1.0), renderer, overwrite );

  mitk::SurfaceVtkMapper3D::SetDefaultPropertiesForVtkProperty(node,renderer,overwrite); // Shading
  
  node->AddProperty( "scalar visibility", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "color mode", mitk::BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "scalar mode", mitk::VtkScalarModeProperty::New(), renderer, overwrite );
  mitk::Surface::Pointer surface = dynamic_cast<Surface*>(node->GetData());
  if(surface.IsNotNull())
  {
    if((surface->GetVtkPolyData() != 0) && (surface->GetVtkPolyData()->GetPointData() != NULL) && (surface->GetVtkPolyData()->GetPointData()->GetScalars() != 0))
    {
      node->AddProperty( "scalar visibility", mitk::BoolProperty::New(true), renderer, overwrite );
      node->AddProperty( "color mode", mitk::BoolProperty::New(true), renderer, overwrite );
    }
  }
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}


void mitk::SurfaceVtkMapper3D::SetImmediateModeRenderingOn(int  /*on*/)
{
/*
  if (m_VtkPolyDataMapper != NULL) 
    m_VtkPolyDataMapper->SetImmediateModeRendering(on);
*/
}
