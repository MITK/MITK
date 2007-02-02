/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkGeometry2DDataVtkMapper3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkDataTree.h"
#include "mitkImageMapper2D.h"
#include "mitkSurface.h"
#include "mitkGeometry2DDataToSurfaceFilter.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkLookupTableProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkSmartPointerProperty.h"
#include "mitkWeakPointerProperty.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkLookupTable.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkAssembly.h>
#include <vtkFeatureEdges.h>
#include <vtkTubeFilter.h>

#include "pic2vtk.h"

//##ModelId=3E691E09038E
mitk::Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D() : m_DataTreeIterator(NULL), m_LastTextureUpdateTime(0)
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_VtkPolyDataMapper->ImmediateModeRenderingOn();

  m_ImageActor = vtkActor::New();
  m_ImageActor->SetMapper(m_VtkPolyDataMapper);
  m_ImageActor->GetProperty()->SetAmbient(0.5);

  m_VtkLookupTable = vtkLookupTable::New();
  m_VtkLookupTable->SetTableRange (-1024, 4096);
  m_VtkLookupTable->SetSaturationRange (0, 0);
  m_VtkLookupTable->SetHueRange (0, 0);
  m_VtkLookupTable->SetValueRange (0, 1);
  m_VtkLookupTable->Build ();

  m_VtkLookupTableDefault = m_VtkLookupTable;

  m_VtkTexture = vtkTexture::New();
  m_VtkTexture->InterpolateOn();
  m_VtkTexture->SetLookupTable(m_VtkLookupTable);
  m_VtkTexture->MapColorScalarsThroughLookupTableOn();

  m_EdgeTuber = vtkTubeFilter::New();
  m_EdgeMapper = vtkPolyDataMapper::New();
  m_Edges = vtkFeatureEdges::New();
  m_EdgeActor = vtkActor::New();

  m_EdgeTuber->SetInput( m_Edges->GetOutput() );
  m_EdgeTuber->SetNumberOfSides( 12 );
  m_EdgeTuber->CappingOn();

  m_EdgeMapper->SetInput( m_EdgeTuber->GetOutput() );
  m_EdgeMapper->ScalarVisibilityOff();

  m_EdgeActor->SetMapper( m_EdgeMapper );

  m_Prop3DAssembly = vtkAssembly::New();
  m_Prop3DAssembly->AddPart( m_ImageActor );
  m_Prop3DAssembly->AddPart( m_EdgeActor );
  
  m_Prop3D = m_Prop3DAssembly;
  m_Prop3D->Register(NULL);
}

//##ModelId=3E691E090394
mitk::Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
  m_VtkPolyDataMapper->Delete();
  m_Prop3DAssembly->Delete();
  m_ImageActor->Delete();
  m_VtkLookupTable->Delete();
  m_VtkTexture->Delete();
  m_EdgeTuber->Delete();
  m_EdgeMapper->Delete();
  m_Edges->Delete();
  m_EdgeActor->Delete();
}


//##ModelId=3E691E090380
const mitk::Geometry2DData *mitk::Geometry2DDataVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}

//##ModelId=3E6E874F0007
void mitk::Geometry2DDataVtkMapper3D::SetDataIteratorForTexture(const mitk::DataTreeIteratorBase* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    m_DataTreeIterator = iterator;
    Modified();
  }
}

//##ModelId=3EF19F850151
void mitk::Geometry2DDataVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  if ( !this->IsVisible(renderer) )
  {
    m_Prop3DAssembly->VisibilityOff();
    // visibility has explicitly to be set in the single actors
    // due to problems when using cell picking:
    // even if the assembly is invisible, the renderer contains 
    // references to the assemblies parts. During picking the
    // visibility of each part is checked, and not only for the
    // whole assembly.
    m_ImageActor->VisibilityOff(); 
    m_EdgeActor->VisibilityOff(); 
    return;
  }

  m_Prop3DAssembly->VisibilityOn();
  // visibility has explicitly to be set in the single actors
  // due to problems when using cell picking:
  // even if the assembly is invisible, the renderer contains 
  // references to the assemblies parts. During picking the
  // visibility of each part is checked, and not only for the
  // whole assembly.
  m_ImageActor->VisibilityOn();
  m_EdgeActor->VisibilityOn();

  mitk::Geometry2DData::Pointer input  = const_cast<mitk::Geometry2DData*>(this->GetInput());

  if(input.IsNotNull() && (input->GetGeometry2D() != NULL))
  {
    mitk::Geometry2DDataToSurfaceFilter::Pointer surfaceCreator;
    mitk::SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop=dynamic_cast<mitk::SmartPointerProperty*>(GetDataTreeNode()->GetProperty("surfacegeometry", renderer).GetPointer());
    if( (surfacecreatorprop.IsNull()) || 
      (surfacecreatorprop->GetSmartPointer().IsNull()) ||
      ((surfaceCreator=dynamic_cast<mitk::Geometry2DDataToSurfaceFilter*>(surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull())
      )
    {
      surfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
      surfaceCreator->PlaceByGeometryOn();
      surfacecreatorprop=new mitk::SmartPointerProperty(surfaceCreator);
      GetDataTreeNode()->SetProperty("surfacegeometry", surfacecreatorprop);
    }

    surfaceCreator->SetInput(input);

    int res;
    if(GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
      surfaceCreator->SetXResolution(res);
    if(GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
      surfaceCreator->SetYResolution(res);

    surfaceCreator->Update(); //@FIXME ohne das crash
    m_VtkPolyDataMapper->SetInput(surfaceCreator->GetOutput()->GetVtkPolyData());

    bool texture=false;
    if(m_DataTreeIterator.IsNotNull())
    {
      mitk::DataTreeIteratorClone it=m_DataTreeIterator.GetPointer();
      while(!it->IsAtEnd())
      {
        mitk::DataTreeNode* node=it->Get();
        if ( node != NULL )
        {
          mitk::Mapper::Pointer mapper = node->GetMapper(1);
          mitk::ImageMapper2D* imagemapper = dynamic_cast<ImageMapper2D*>(mapper.GetPointer());

          if((node->IsVisible(renderer)) && (imagemapper))
          {
            mitk::WeakPointerProperty::Pointer rendererProp 
                                                = dynamic_cast<mitk::WeakPointerProperty*>(GetDataTreeNode()->GetPropertyList()->GetProperty("renderer").GetPointer());
            if(rendererProp.IsNotNull())
            {
              mitk::BaseRenderer::Pointer renderer = dynamic_cast<mitk::BaseRenderer*>(rendererProp->GetWeakPointer().GetPointer());
              if(renderer.IsNotNull())
              {
                bool useColor = true;
                
                // check for "use color"
                node->GetBoolProperty("use color", useColor, renderer);
                
                // check for LookupTable
                mitk::LookupTableProperty::Pointer lookupTableProp;
                lookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(node->GetPropertyList()->GetProperty("LookupTable").GetPointer());
                if (lookupTableProp.IsNotNull() && !useColor)
                {
                  m_VtkLookupTable = lookupTableProp->GetLookupTable()->GetVtkLookupTable();
                  m_VtkTexture->SetLookupTable(m_VtkLookupTable);
                  // m_VtkTexture->Modified();
                } else {
                  m_VtkLookupTable = m_VtkLookupTableDefault;
                }


                // check for level window prop and use it for display if it exists
                mitk::LevelWindow levelWindow;
                if(node->GetLevelWindow(levelWindow, renderer))
                  m_VtkLookupTable->SetTableRange(levelWindow.GetMin(),levelWindow.GetMax());


                //we have to do this before GenerateAllData() is called there may be
                //no RendererInfo for renderer yet, thus GenerateAllData won't update
                //the (non-existing) RendererInfo for renderer. By calling GetRendererInfo
                //a RendererInfo will be created for renderer (if it does not exist yet).
                const ImageMapper2D::RendererInfo* ri=imagemapper->GetRendererInfo(renderer);
                imagemapper->GenerateAllData();
                texture = true;
                if((ri!=NULL) && (ri->m_Pic!=NULL) &&(m_LastTextureUpdateTime<ri->m_LastUpdateTime))
                {
                  ipPicDescriptor *p=ri->m_Pic;
                  if((p->dim==2) && (p->n[0]>2) && (p->n[1]>2))
                  {
                    vtkImageData* vtkimage=Pic2vtk::convert(p);
                    m_VtkTexture->SetInput(vtkimage);
                    vtkimage->Delete(); vtkimage=NULL;
                    m_ImageActor->SetTexture(m_VtkTexture);
                    m_LastTextureUpdateTime=ri->m_LastUpdateTime;
                    bool textureInterpolation=true;
                    m_VtkTexture->SetInterpolate(textureInterpolation ? 1 : 0);
                  }
                  else
                    texture = false;
                }
                break;
              }
            }
          }
        }
        ++it;
      }
    }
    if(texture==false)
    {
      m_ImageActor->SetTexture(NULL);
    }

    // Configurate the tube-shaped frame: size according to the surface
    // bounds, color as specified in the plane's properties
    vtkPolyData* surfacePolyData = surfaceCreator->GetOutput()->GetVtkPolyData();
    m_Edges->SetInput( surfacePolyData );

    // Determine maximum extent
    vtkFloatingPointType* surfaceBounds = surfacePolyData->GetBounds();
    vtkFloatingPointType extent = surfaceBounds[1] - surfaceBounds[0];
    vtkFloatingPointType extentY = surfaceBounds[3] - surfaceBounds[2];
    vtkFloatingPointType extentZ = surfaceBounds[5] - surfaceBounds[4];
    
    if ( extent < extentY ) extent = extentY;
    if ( extent < extentZ ) extent = extentZ;

    // Adjust the radius according to extent
    m_EdgeTuber->SetRadius( extent / 250.0 );

    // Get the plane's color and set the tube properties accordingly
    mitk::ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<mitk::ColorProperty*>(
      this->GetDataTreeNode()->GetProperty("color").GetPointer( )
    );
    if ( colorProperty.IsNotNull() )
    {
      const mitk::Color& color = colorProperty->GetColor();
      m_EdgeActor->GetProperty()->SetColor(
        color.GetRed(), color.GetGreen(), color.GetBlue()
      );
    }
    else
    {
      m_EdgeActor->GetProperty()->SetColor( 1.0, 1.0, 1.0 );
    }

    // Apply properties read from the PropertyList
    this->ApplyProperties(m_ImageActor, renderer);

    m_Prop3D->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
  }
}
