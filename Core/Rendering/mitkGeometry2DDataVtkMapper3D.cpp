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
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyData.h>
#include <vtkLookupTable.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkAssembly.h>
#include <vtkFeatureEdges.h>
#include <vtkTubeFilter.h>
#include <vtkMath.h>
#include <vtkProp3DCollection.h>

#include <vtkImageShrink3D.h>
#include <vtkImageConstantPad.h>
#include <vtkImageChangeInformation.h>
#include <vtkTransformPolyDataFilter.h>

#include "pic2vtk.h"

namespace mitk
{

Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D()
: m_DataTreeIterator(NULL)
{
  m_EdgeTuber = vtkTubeFilter::New();
  m_EdgeMapper = vtkPolyDataMapper::New();

  m_SurfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
  m_Edges = vtkFeatureEdges::New();
  m_EdgeTransformer = vtkTransformPolyDataFilter::New();
  m_EdgeActor = vtkActor::New();
  m_BackgroundMapper = vtkDataSetMapper::New();
  m_BackgroundActor = vtkActor::New();
  m_Prop3DAssembly = vtkAssembly::New();
  m_ImageAssembly = vtkAssembly::New();

  // Make sure that the FeatureEdge algorithm is initialized with a "valid"
  // (though empty) input
  vtkPolyData *emptyPolyData = vtkPolyData::New();
  m_Edges->SetInput( emptyPolyData );
  emptyPolyData->Delete(); // decrease reference count

  m_EdgeTransformer->SetInput( m_Edges->GetOutput() );

  m_EdgeTuber->SetInput( m_EdgeTransformer->GetOutput() );
  m_EdgeTuber->SetVaryRadiusToVaryRadiusOff();
  m_EdgeTuber->SetNumberOfSides( 12 );
  m_EdgeTuber->CappingOn();

  m_EdgeMapper->SetInput( m_EdgeTuber->GetOutput() );
  m_EdgeMapper->ScalarVisibilityOff();

  m_EdgeActor->SetMapper( m_EdgeMapper );


  m_BackgroundMapper->ImmediateModeRenderingOn();

  m_BackgroundActor->GetProperty()->SetAmbient( 0.5 );
  m_BackgroundActor->GetProperty()->SetColor( 0.0, 0.0, 0.0 );
  m_BackgroundActor->GetProperty()->SetOpacity( 1.0 );
  m_BackgroundActor->SetMapper( m_BackgroundMapper );

  m_Prop3DAssembly->AddPart( m_EdgeActor );
  m_Prop3DAssembly->AddPart( m_ImageAssembly );
  
  m_Prop3D = m_Prop3DAssembly;
  m_Prop3D->Register(NULL);

  m_DefaultLookupTable = vtkLookupTable::New();
  m_DefaultLookupTable->SetTableRange( -1024.0, 4096.0 );
  m_DefaultLookupTable->SetSaturationRange( 0.0, 0.0 );
  m_DefaultLookupTable->SetHueRange( 0.0, 0.0 );
  m_DefaultLookupTable->SetValueRange( 0.0, 1.0 );
  m_DefaultLookupTable->Build();
  m_DefaultLookupTable->SetTableValue( 0, 0.0, 0.0, 0.0, 0.0 );
}


Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
  m_ImageAssembly->Delete();
  m_Prop3DAssembly->Delete();
  m_EdgeTuber->Delete();
  m_EdgeMapper->Delete();
  m_EdgeTransformer->Delete();
  m_Edges->Delete();
  m_EdgeActor->Delete();
  m_BackgroundMapper->Delete();
  m_BackgroundActor->Delete();

  // Delete entries in m_ImageActors list one by one
  ActorList::iterator it;

  for ( it = m_ImageActors.begin(); it != m_ImageActors.end(); ++it )
  {
    it->second->Delete();
  }
}


vtkProp *
Geometry2DDataVtkMapper3D::GetProp()
{
  if ( (this->GetDataTreeNode() != NULL )
       && (m_Prop3D != NULL) 
       && (m_ImageAssembly != NULL) ) 
  {
    // Do not transform the entire Prop3D assembly, but only the image part
    // here. The colored frame is transformed elsewhere (via m_EdgeTransformer),
    // since only vertices should be transformed there, not the poly data
    // itself, to avoid distortion for anisotropic datasets.
    m_ImageAssembly->SetUserTransform( this->GetDataTreeNode()->GetVtkTransform() );
  }
 
  return m_Prop3D;
}

void mitk::Geometry2DDataVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer* renderer)
{
  m_ImageAssembly->SetUserTransform( 
    this->GetDataTreeNode()->GetVtkTransform(renderer->GetTimeStep(
      this->GetDataTreeNode()->GetData())) );
}

const Geometry2DData *
Geometry2DDataVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Geometry2DData * > ( GetData() );
}


void 
Geometry2DDataVtkMapper3D
::SetDataIteratorForTexture(const mitk::DataTreeIteratorBase* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    m_DataTreeIterator = iterator;
    this->Modified();
  }
}

//
// METHOD COMMENTED OUT SINCE IT IS CURRENTLY UNUSED
//
//void
//Geometry2DDataVtkMapper3D::BuildPaddedLookupTable( 
//  vtkLookupTable *inputLookupTable, vtkLookupTable *outputLookupTable,
//  vtkFloatingPointType min, vtkFloatingPointType max )
//{
//  // Copy the table values from the input lookup table
//  vtkUnsignedCharArray *inputTable = vtkUnsignedCharArray::New();
//  inputTable->DeepCopy( inputLookupTable->GetTable() );
//
//  vtkUnsignedCharArray *outputTable = outputLookupTable->GetTable();
//
//  // Calculate the size of one lookup table "bin"
//  vtkFloatingPointType binSize = (max - min) / 256.0;
//
//  // Calculate the extended table size, assuming the range [-32767, max],
//  // increased by 1.
//  int tableSize = (int) ((max + 32767) / binSize) + 1;
//  outputLookupTable->SetNumberOfTableValues( tableSize );
//
//  unsigned char *inputPtr = inputTable->WritePointer( 0, 0 );
//  unsigned char *outputPtr = outputTable->WritePointer( 0, 0 );
//
//  // Initialize the first (translucent) bin.
//  *outputPtr++ = 0; *outputPtr++ = 0; *outputPtr++ = 0; *outputPtr++ = 0; 
//  
//  int i;
//  for ( i = 1; i < tableSize; ++i )
//  {
//    *outputPtr++ = *inputPtr++;
//    *outputPtr++ = *inputPtr++;
//    *outputPtr++ = *inputPtr++;
//    *outputPtr++ = *inputPtr++;
//
//    // While filling the padded part of the table, use the default value
//    // (the first value of the input table)
//    if ( i < (tableSize - 256) )
//    {
//      inputPtr -= 4;
//    }
//  }
//
//  // Apply the new table range; the lower boundary is decreased by binSize
//  // since we have one additional translucent bin.
//  outputLookupTable->SetTableRange( -32767.0 - binSize, max );
//
//  inputTable->Delete();
//}


int
Geometry2DDataVtkMapper3D::FindPowerOfTwo( int i )
{
  int size;

  for ( --i, size = 1; i > 0; size *= 2 )
  {
    i /= 2;
  }
  return size;
}


void 
Geometry2DDataVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  // Remove all actors from the assembly, and re-initialize it with the 
  // edge actor
  m_ImageAssembly->GetParts()->RemoveAllItems();

  if ( !this->IsVisible(renderer) )
  {
    // visibility has explicitly to be set in the single actors
    // due to problems when using cell picking:
    // even if the assembly is invisible, the renderer contains 
    // references to the assemblies parts. During picking the
    // visibility of each part is checked, and not only for the
    // whole assembly.
    m_ImageAssembly->VisibilityOff();
    m_EdgeActor->VisibilityOff(); 
    return;
  }

  // visibility has explicitly to be set in the single actors
  // due to problems when using cell picking:
  // even if the assembly is invisible, the renderer contains 
  // references to the assemblies parts. During picking the
  // visibility of each part is checked, and not only for the
  // whole assembly.
  m_ImageAssembly->VisibilityOn();
  m_EdgeActor->VisibilityOn();

  mitk::Geometry2DData::Pointer input =
    const_cast< mitk::Geometry2DData * >(this->GetInput());

  if (input.IsNotNull() && (input->GetGeometry2D() != NULL))
  {
    mitk::SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop = dynamic_cast< mitk::SmartPointerProperty * >(
      GetDataTreeNode()->GetProperty("surfacegeometry", renderer).GetPointer()
    );

    if ( (surfacecreatorprop.IsNull())
      || (surfacecreatorprop->GetSmartPointer().IsNull())
      || ((m_SurfaceCreator = dynamic_cast<mitk::Geometry2DDataToSurfaceFilter*>(
             surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull() )
      )
    {
      m_SurfaceCreator = mitk::Geometry2DDataToSurfaceFilter::New();
      m_SurfaceCreator->PlaceByGeometryOn();
      surfacecreatorprop = new mitk::SmartPointerProperty( m_SurfaceCreator );
      GetDataTreeNode()->SetProperty("surfacegeometry", surfacecreatorprop);
    }

    m_SurfaceCreator->SetInput(input);

    int res;
    if (GetDataTreeNode()->GetIntProperty("xresolution", res, renderer))
    {
      m_SurfaceCreator->SetXResolution(res);
    }
    if (GetDataTreeNode()->GetIntProperty("yresolution", res, renderer))
    {
      m_SurfaceCreator->SetYResolution(res);
    }
    
    // Clip the Geometry2D with the reference geometry bounds (if available)
    if ( input->GetGeometry2D()->HasReferenceGeometry() )
    {
      m_SurfaceCreator->SetBoundingBox(
        input->GetGeometry2D()->GetReferenceGeometry()->GetBoundingBox()
      );
    }
    else
    {
      // If no reference geometry is available, clip with the current global
      // bounds
      m_SurfaceCreator->SetBoundingBox(
        mitk::DataTree::ComputeVisibleBoundingBox(
          m_DataTreeIterator.GetPointer(), NULL, "includeInBoundingBox"
        )
      );
    }

    // Calculate the surface of the Geometry2D
    m_SurfaceCreator->Update();

    mitk::Surface *surface = m_SurfaceCreator->GetOutput();

    // Check if there's something to display, otherwise return
    if ( (surface->GetVtkPolyData() == 0 )
      || (surface->GetVtkPolyData()->GetNumberOfCells() == 0) )
    {
      m_ImageAssembly->VisibilityOff();
      return;
    }


    // Add black background for all images (which may be transparent)
    m_BackgroundMapper->SetInput( m_SurfaceCreator->GetOutput()->GetVtkPolyData() );
    m_ImageAssembly->AddPart( m_BackgroundActor );


    LayerSortedActorList layerSortedActors;


    // Traverse the data tree to find nodes resliced by ImageMapper2D
    if ( m_DataTreeIterator.IsNotNull() )
    {
      mitk::DataTreeIteratorClone it = m_DataTreeIterator.GetPointer();
      int nodeCounter = 0;

      while ( !it->IsAtEnd() )
      {
        mitk::DataTreeNode *node = it->Get();

        if ( node != NULL )
        {
          mitk::ImageMapper2D *imageMapper =
            dynamic_cast< ImageMapper2D * >( node->GetMapper(1) );

          if ( (node->IsVisible(renderer)) && imageMapper )
          {
            nodeCounter++;
            mitk::WeakPointerProperty::Pointer rendererProp =
              dynamic_cast< mitk::WeakPointerProperty * >(
                GetDataTreeNode()->GetPropertyList()
                  ->GetProperty("renderer").GetPointer()
              );

            if ( rendererProp.IsNotNull() )
            {
              mitk::BaseRenderer::Pointer planeRenderer =
                dynamic_cast< mitk::BaseRenderer * >(
                  rendererProp->GetWeakPointer().GetPointer()
                );

              if ( planeRenderer.IsNotNull() )
              {
                // If it has not been initialized already in a previous pass,
                // generate an actor, a lookup table and a texture object to
                // render the image associated with the ImageMapper2D.
                vtkActor *imageActor;
                vtkLookupTable *lookupTable;
                vtkTexture *texture;
                if ( m_ImageActors.count( imageMapper ) == 0 )
                {
                  vtkDataSetMapper *dataSetMapper = vtkDataSetMapper::New();
                  dataSetMapper->ImmediateModeRenderingOn();
                  dataSetMapper->SetInput( 
                    m_SurfaceCreator->GetOutput()->GetVtkPolyData() );

                  lookupTable = vtkLookupTable::New();
                  lookupTable->DeepCopy( m_DefaultLookupTable );
                  lookupTable->SetRange( -1024.0, 4095.0 );

                  texture = vtkTexture::New();
                  texture->InterpolateOn();
                  texture->SetLookupTable( lookupTable );
                  texture->MapColorScalarsThroughLookupTableOn();
                  texture->RepeatOff();

                  imageActor = vtkActor::New();
                  imageActor->GetProperty()->SetAmbient( 0.5 );
                  imageActor->SetMapper( dataSetMapper );
                  imageActor->SetTexture( texture );

                  // Make imageActor the sole owner of the mapper and texture
                  // objects
                  dataSetMapper->Delete();
                  texture->Delete();
                  
                  // Store the actor so that it may be accessed in following
                  // passes.
                  m_ImageActors[imageMapper] = imageActor;
                }
                else
                {
                  // Else, retrieve the actor and associated objects from the
                  // previous pass.
                  imageActor = m_ImageActors[imageMapper];
                  texture = imageActor->GetTexture();
                  lookupTable = texture->GetLookupTable();
                }

                // We have to do this before GenerateAllData() is called
                // since there may be no RendererInfo for renderer yet,
                // thus GenerateAllData won't update the (non-existing)
                // RendererInfo for renderer. By calling GetRendererInfo
                // a RendererInfo will be created for renderer (if it does not
                // exist yet).
                imageMapper->GetRendererInfo( planeRenderer );

                imageMapper->GenerateAllData();

                // Retrieve and update image to be mapped
                const ImageMapper2D::RendererInfo *rit =
                  imageMapper->GetRendererInfo( planeRenderer );
                if(rit->m_Image!=NULL)
                {
                  rit->m_Image->Update();
                  texture->SetInput( rit->m_Image );


                  // check for level-window-prop and use it if it exists
                  mitk::ScalarType windowMin = 0.0;
                  mitk::ScalarType windowMax = 255.0;
                  mitk::LevelWindow levelWindow;

                  bool binary = false;
                  node->GetBoolProperty( "binary", binary, renderer );

                  if( binary )
                  {
                    windowMin = 0;
                    windowMax = 1;
                  }
                  else
                  if( node->GetLevelWindow( levelWindow, planeRenderer, "levelWindow" ) 
                    || node->GetLevelWindow( levelWindow, planeRenderer ) )
                  {
                    windowMin = levelWindow.GetMin();
                    windowMax = levelWindow.GetMax();
                  }

                  vtkLookupTable *lookupTableSource;

                  // check for "use color"
                  bool useColor = true;
                  if ( !node->GetBoolProperty( "use color", useColor, planeRenderer ) )
                  {
                    useColor = false;
                  }
                  
                  // check for LookupTable
                  mitk::LookupTableProperty::Pointer lookupTableProp;
                  lookupTableProp = dynamic_cast< mitk::LookupTableProperty * >(
                    node->GetPropertyList()
                      ->GetProperty( "LookupTable" ).GetPointer()
                  );

                  // If there is a lookup table supplied, use it; otherwise,
                  // use the default grayscale table
                  if ( lookupTableProp.IsNotNull()  && !useColor )
                  {
                    lookupTableSource = lookupTableProp->GetLookupTable()
                      ->GetVtkLookupTable();
                  }
                  else
                  {
                    lookupTableSource = m_DefaultLookupTable;
                  }

                  LookupTableProperties &lutProperties = 
                    m_LookupTableProperties[imageMapper];

                  // If there has been some change since the last pass which
                  // makes it necessary to re-build the lookup table, do it.
                  if ( (lutProperties.LookupTableSource != lookupTableSource)
                    || (lutProperties.windowMin != windowMin)
                    || (lutProperties.windowMax != windowMax) )
                  {
                    // Note the values for the next pass (lutProperties is a 
                    // reference to the list entry!)
                    lutProperties.LookupTableSource = lookupTableSource;
                    lutProperties.windowMin = windowMin;
                    lutProperties.windowMax = windowMax;
                    
                    lookupTable->DeepCopy( lookupTableSource );
                    lookupTable->SetRange( windowMin, windowMax );
                  }


                  // We have to do this before GenerateAllData() is called
                  // since there may be no RendererInfo for renderer yet,
                  // thus GenerateAllData won't update the (non-existing)
                  // RendererInfo for renderer. By calling GetRendererInfo
                  // a RendererInfo will be created for renderer (if it does not
                  // exist yet).
                  //const ImageMapper2D::RendererInfo *ri = 
                  //  imageMapper->GetRendererInfo( planeRenderer );

                  //imageMapper->GenerateAllData();


                  // Apply color property (of the node, not of the plane)
                  float rgb[3] = { 1.0, 1.0, 1.0 };
                  node->GetColor( rgb, renderer );
                  imageActor->GetProperty()->SetColor( rgb[0], rgb[1], rgb[2] );


                  // Apply opacity property (of the node, not of the plane)
                  float opacity = 0.999;
                  node->GetOpacity( opacity, renderer );
                  imageActor->GetProperty()->SetOpacity( opacity );

                  // Set texture interpolation on/off
                  bool textureInterpolation = node->IsOn( "texture interpolation", renderer );
                  texture->SetInterpolate( textureInterpolation );


                  // Store this actor to be added to the actor assembly, sort
                  // by layer
                  int layer = 1;
                  node->GetIntProperty( "layer", layer );
                  layerSortedActors.insert( 
                    std::pair< int, vtkActor * >( layer, imageActor ) );
                }
              }
            }
          }
        }
        ++it;
      }
    }


    // Add all image actors to the assembly, sorted according to 
    // layer property
    LayerSortedActorList::iterator actorIt;
    for ( actorIt = layerSortedActors.begin(); 
          actorIt != layerSortedActors.end();
          ++actorIt )
    {
      m_ImageAssembly->AddPart( actorIt->second );
    }


    // Configurate the tube-shaped frame: size according to the surface
    // bounds, color as specified in the plane's properties
    vtkPolyData *surfacePolyData = surface->GetVtkPolyData();

    m_Edges->SetInput( surfacePolyData );

    m_EdgeTransformer->SetTransform( 
      this->GetDataTreeNode()->GetVtkTransform(
        renderer->GetTimeStep( this->GetDataTreeNode()->GetData())) );

    // Determine maximum extent
    vtkFloatingPointType* surfaceBounds = surfacePolyData->GetBounds();
    vtkFloatingPointType extent = surfaceBounds[1] - surfaceBounds[0];
    vtkFloatingPointType extentY = surfaceBounds[3] - surfaceBounds[2];
    vtkFloatingPointType extentZ = surfaceBounds[5] - surfaceBounds[4];
    
    if ( extent < extentY ) extent = extentY;
    if ( extent < extentZ ) extent = extentZ;

    // Adjust the radius according to extent
    m_EdgeTuber->SetRadius( extent / 450.0 );

    // Get the plane's color and set the tube properties accordingly
    mitk::ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<mitk::ColorProperty*>(
      this->GetDataTreeNode()->GetProperty( "color" ).GetPointer( )
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

    m_ImageAssembly->SetUserTransform( 
      this->GetDataTreeNode()->GetVtkTransform(renderer->GetTimeStep(
        this->GetDataTreeNode()->GetData())) );
  }
}

} // namespace mitk
