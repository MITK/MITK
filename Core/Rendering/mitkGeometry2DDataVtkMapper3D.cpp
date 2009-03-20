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


#include "mitkGeometry2DDataVtkMapper3D.h"

#include "mitkDataTreeStorage.h"
#include "mitkDataStorage.h"
#include "mitkImageMapper2D.h"
#include "mitkLookupTableProperty.h"
#include "mitkSmartPointerProperty.h"
#include "mitkSurface.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkWeakPointerProperty.h"

#include <vtkAssembly.h>
#include <vtkDataSetMapper.h>
#include <vtkFeatureEdges.h>
#include <vtkHedgeHog.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkLookupTable.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>
#include "pic2vtk.h"

namespace mitk
{

Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D()
: m_DisplayNormals(false),
  m_ColorTwoSides(false),
  m_InvertNormals(true),
  m_NormalsActorAdded(false),
  m_DataTreeIterator(NULL)
{
  m_EdgeTuber = vtkTubeFilter::New();
  m_EdgeMapper = vtkPolyDataMapper::New();
  
  // Disable OGL Displaylist for the "Edge". Workaround for Bug #1787
  m_EdgeMapper->ImmediateModeRenderingOn();

  m_SurfaceCreator = Geometry2DDataToSurfaceFilter::New();
  m_SurfaceCreatorBoundingBox = BoundingBox::New();
  m_SurfaceCreatorPointsContainer = BoundingBox::PointsContainer::New();
  m_Edges = vtkFeatureEdges::New();

  m_Edges->BoundaryEdgesOn();
  m_Edges->FeatureEdgesOff();
  m_Edges->NonManifoldEdgesOff();
  m_Edges->ManifoldEdgesOff();

  m_EdgeTransformer = vtkTransformPolyDataFilter::New();
  m_NormalsTransformer = vtkTransformPolyDataFilter::New();
  m_EdgeActor = vtkActor::New();
  m_BackgroundMapper = vtkPolyDataMapper::New();
  m_BackgroundActor = vtkActor::New();
  m_Prop3DAssembly = vtkAssembly::New();
  m_ImageAssembly = vtkAssembly::New();

  m_SurfaceCreatorBoundingBox->SetPoints( m_SurfaceCreatorPointsContainer );

  m_Cleaner = vtkCleanPolyData::New();

  m_Cleaner->PieceInvariantOn();
  m_Cleaner->ConvertLinesToPointsOn();
  m_Cleaner->ConvertPolysToLinesOn();
  m_Cleaner->ConvertStripsToPolysOn();
  m_Cleaner->PointMergingOn();

  // Make sure that the FeatureEdge algorithm is initialized with a "valid"
  // (though empty) input
  vtkPolyData *emptyPolyData = vtkPolyData::New();
  m_Cleaner->SetInput( emptyPolyData );
  emptyPolyData->Delete();

  m_Edges->SetInput(m_Cleaner->GetOutput());
  m_EdgeTransformer->SetInput( m_Edges->GetOutput() );

  m_EdgeTuber->SetInput( m_EdgeTransformer->GetOutput() );
  m_EdgeTuber->SetVaryRadiusToVaryRadiusOff();
  m_EdgeTuber->SetNumberOfSides( 12 );
  m_EdgeTuber->CappingOn();

  m_EdgeMapper->SetInput( m_EdgeTuber->GetOutput() );
  m_EdgeMapper->ScalarVisibilityOff();

  m_EdgeActor->SetMapper( m_EdgeMapper );

  m_BackgroundMapper->SetInput(emptyPolyData);
  m_BackgroundMapper->ImmediateModeRenderingOn();

  m_BackgroundActor->GetProperty()->SetAmbient( 0.5 );
  m_BackgroundActor->GetProperty()->SetColor( 0.0, 0.0, 0.0 );
  m_BackgroundActor->GetProperty()->SetOpacity( 1.0 );
  m_BackgroundActor->SetMapper( m_BackgroundMapper );

  vtkProperty * backfaceProperty = m_BackgroundActor->MakeProperty();
  backfaceProperty->SetColor( 0.0, 0.0, 0.0 );
  m_BackgroundActor->SetBackfaceProperty( backfaceProperty );
  backfaceProperty->Delete();

  m_FrontHedgeHog = vtkHedgeHog::New();
  m_BackHedgeHog  = vtkHedgeHog::New();

  m_FrontNormalsMapper = vtkPolyDataMapper::New();
  m_FrontNormalsMapper->SetInput( m_FrontHedgeHog->GetOutput() );
  m_BackNormalsMapper = vtkPolyDataMapper::New();
  m_BackNormalsMapper->SetInput( m_BackHedgeHog->GetOutput() );

  m_FrontNormalsActor = vtkActor::New();
  m_FrontNormalsActor->SetMapper(m_FrontNormalsMapper);
  m_BackNormalsActor = vtkActor::New();
  m_BackNormalsActor->SetMapper(m_BackNormalsMapper);

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


  m_ImageMapperDeletedCommand = MemberCommandType::New();

  m_ImageMapperDeletedCommand->SetCallbackFunction(
    this, &Geometry2DDataVtkMapper3D::ImageMapperDeletedCallback );
}


Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
  m_ImageAssembly->Delete();
  m_Prop3DAssembly->Delete();
  m_EdgeTuber->Delete();
  m_EdgeMapper->Delete();
  m_EdgeTransformer->Delete();
  m_Cleaner->Delete();
  m_Edges->Delete();
  m_NormalsTransformer->Delete();
  m_EdgeActor->Delete();
  m_BackgroundMapper->Delete();
  m_BackgroundActor->Delete();
  m_DefaultLookupTable->Delete();
  m_FrontNormalsMapper->Delete();
  m_FrontNormalsActor->Delete();
  m_FrontHedgeHog->Delete();
  m_BackNormalsMapper->Delete();
  m_BackNormalsActor->Delete();
  m_BackHedgeHog->Delete();

  // Delete entries in m_ImageActors list one by one
  m_ImageActors.clear();

  LookupTablePropertiesList::iterator it;
  for(it = m_LookupTableProperties.begin(); it != m_LookupTableProperties.end();++it)
  {
    if ( it->second.LookupTableSource != NULL )
    {
      it->second.LookupTableSource->Delete();
      it->second.LookupTableSource = NULL;
    }
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

void Geometry2DDataVtkMapper3D::UpdateVtkTransform()
{
  m_ImageAssembly->SetUserTransform(
    this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep()) );
}

const Geometry2DData *
Geometry2DDataVtkMapper3D::GetInput()
{
  return static_cast<const Geometry2DData * > ( GetData() );
}


void
Geometry2DDataVtkMapper3D
::SetDataIteratorForTexture(const DataTreeIteratorBase* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    m_DataTreeIterator = iterator;
    this->Modified();
  }
}

void
Geometry2DDataVtkMapper3D
::SetDataIteratorForTexture(DataStorage* storage)
{
  if (storage != NULL)
  {
    DataTreePreOrderIterator poi( (dynamic_cast<mitk::DataTreeStorage*>(storage))->m_DataTree );
    this->SetDataIteratorForTexture( &poi );
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
Geometry2DDataVtkMapper3D::ImageMapperDeletedCallback(
  itk::Object *caller, const itk::EventObject& /*event*/ )
{
  ImageMapper2D *imageMapper = dynamic_cast< ImageMapper2D * >( caller );
  if ( (imageMapper != NULL) )
  {
    if ( m_ImageActors.count( imageMapper ) > 0)
    {
      m_ImageActors[imageMapper].m_Sender = NULL; // sender is already destroying itself
      m_ImageActors.erase( imageMapper );
    }
    if ( m_LookupTableProperties.count( imageMapper ) > 0 )
    {
      m_LookupTableProperties[imageMapper].LookupTableSource->Delete();
      m_LookupTableProperties.erase( imageMapper );
    }
  }
}

void
Geometry2DDataVtkMapper3D::GenerateData(BaseRenderer* renderer)
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

  Geometry2DData::Pointer input =
    const_cast< Geometry2DData * >(this->GetInput());

  if (input.IsNotNull() && (input->GetGeometry2D() != NULL))
  {
    SmartPointerProperty::Pointer surfacecreatorprop;
    surfacecreatorprop = dynamic_cast< SmartPointerProperty * >(
      GetDataTreeNode()->GetProperty("surfacegeometry", renderer));

    if ( (surfacecreatorprop.IsNull())
      || (surfacecreatorprop->GetSmartPointer().IsNull())
      || ((m_SurfaceCreator = dynamic_cast<Geometry2DDataToSurfaceFilter*>(
             surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull() )
      )
    {
      //m_SurfaceCreator = Geometry2DDataToSurfaceFilter::New();
      m_SurfaceCreator->PlaceByGeometryOn();
      surfacecreatorprop = SmartPointerProperty::New( m_SurfaceCreator );
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
      Geometry3D *referenceGeometry =
        input->GetGeometry2D()->GetReferenceGeometry();

      BoundingBox::PointType boundingBoxMin, boundingBoxMax;
      boundingBoxMin = referenceGeometry->GetBoundingBox()->GetMinimum();
      boundingBoxMax = referenceGeometry->GetBoundingBox()->GetMaximum();

      if ( referenceGeometry->GetImageGeometry() )
      {
        for ( unsigned int i = 0; i < 3; ++i )
        {
          boundingBoxMin[i] -= 0.5;
          boundingBoxMax[i] -= 0.5;
        }
      }

      m_SurfaceCreatorPointsContainer->CreateElementAt( 0 ) = boundingBoxMin;
      m_SurfaceCreatorPointsContainer->CreateElementAt( 1 ) = boundingBoxMax;

      m_SurfaceCreatorBoundingBox->ComputeBoundingBox();

      m_SurfaceCreator->SetBoundingBox( m_SurfaceCreatorBoundingBox );
    }
    else
    {
      // If no reference geometry is available, clip with the current global
      // bounds
      m_SurfaceCreator->SetBoundingBox(
        DataTree::ComputeVisibleBoundingBox(
          m_DataTreeIterator.GetPointer(), NULL, "includeInBoundingBox"
        )
      );
    }

    // Calculate the surface of the Geometry2D
    m_SurfaceCreator->Update();

    Surface *surface = m_SurfaceCreator->GetOutput();

    // Check if there's something to display, otherwise return
    if ( (surface->GetVtkPolyData() == 0 )
      || (surface->GetVtkPolyData()->GetNumberOfCells() == 0) )
    {
      m_ImageAssembly->VisibilityOff();
      return;
    }

    // add a graphical representation of the surface normals if requested
    DataTreeNode* node = this->GetDataTreeNode();
    node->GetBoolProperty("draw normals 3D", m_DisplayNormals, renderer);
    node->GetBoolProperty("color two sides", m_ColorTwoSides, renderer);
    node->GetBoolProperty("invert normals", m_InvertNormals, renderer);

    if ( m_DisplayNormals || m_ColorTwoSides )
    {
      float frontColor[3] = { 0.0, 0.0, 1.0 };
      node->GetColor( frontColor, renderer, "front color" );
      float backColor[3] = { 1.0, 0.0, 0.0 };
      node->GetColor( backColor, renderer, "back color" );

      if ( m_DisplayNormals )
      {
        m_NormalsTransformer->SetInput( surface->GetVtkPolyData() );
        m_NormalsTransformer->SetTransform(
          node->GetVtkTransform(this->GetTimestep()) );

        m_FrontHedgeHog->SetInput( m_NormalsTransformer->GetOutput() );
        m_FrontHedgeHog->SetVectorModeToUseNormal();
        m_FrontHedgeHog->SetScaleFactor( m_InvertNormals ? 1.0 : -1.0 );

        m_FrontNormalsActor->GetProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );

        m_BackHedgeHog->SetInput( m_NormalsTransformer->GetOutput() );
        m_BackHedgeHog->SetVectorModeToUseNormal();
        m_BackHedgeHog->SetScaleFactor( m_InvertNormals ? -1.0 : 1.0 );

        m_BackNormalsActor->GetProperty()->SetColor( backColor[0], backColor[1], backColor[2] );

        if ( !m_NormalsActorAdded )
        {
          m_Prop3DAssembly->AddPart( m_FrontNormalsActor );
          m_Prop3DAssembly->AddPart( m_BackNormalsActor );
          m_NormalsActorAdded = true;
        }
      }

      if ( m_ColorTwoSides )
      {
        if ( !m_InvertNormals )
        {
          m_BackgroundActor->GetBackfaceProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );
          m_BackgroundActor->GetProperty()->SetColor( backColor[0], backColor[1], backColor[2] );
        }
        else
        {
          m_BackgroundActor->GetProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );
          m_BackgroundActor->GetBackfaceProperty()->SetColor( backColor[0], backColor[1], backColor[2] );
        }
      }

    }
    else if ( !m_DisplayNormals )
    {
      if ( m_NormalsActorAdded )
      {
        m_Prop3DAssembly->RemovePart( m_FrontNormalsActor );
        m_Prop3DAssembly->RemovePart( m_BackNormalsActor );
        m_NormalsActorAdded = false;
      }
    }


    // Add black background for all images (which may be transparent)
    m_BackgroundMapper->SetInput( surface->GetVtkPolyData() );
    m_ImageAssembly->AddPart( m_BackgroundActor );


    LayerSortedActorList layerSortedActors;


    // Traverse the data tree to find nodes resliced by ImageMapper2D
    if ( m_DataTreeIterator.IsNotNull() )
    {
      DataTreeIteratorClone it = m_DataTreeIterator.GetPointer();
      int nodeCounter = 0;

      while ( !it->IsAtEnd() )
      {
        DataTreeNode *node = it->Get();

        if ( node != NULL )
        {
          ImageMapper2D *imageMapper =
            dynamic_cast< ImageMapper2D * >( node->GetMapper(1) );

          if ( (node->IsVisible(renderer)) && imageMapper )
          {
            nodeCounter++;
            WeakPointerProperty::Pointer rendererProp =
              dynamic_cast< WeakPointerProperty * >(
                GetDataTreeNode()->GetPropertyList()
                  ->GetProperty("renderer"));

            if ( rendererProp.IsNotNull() )
            {
              BaseRenderer::Pointer planeRenderer =
                dynamic_cast< BaseRenderer * >(
                  rendererProp->GetWeakPointer().GetPointer()
                );

              if ( planeRenderer.IsNotNull() )
              {
                // If it has not been initialized already in a previous pass,
                // generate an actor, a lookup table and a texture object to
                // render the image associated with the ImageMapper2D.
                vtkActor *imageActor;
                vtkDataSetMapper *dataSetMapper = NULL;
                vtkLookupTable *lookupTable;
                vtkTexture *texture;
                if ( m_ImageActors.count( imageMapper ) == 0 )
                {
                  dataSetMapper = vtkDataSetMapper::New();
                  dataSetMapper->ImmediateModeRenderingOn();

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
                  lookupTable->UnRegister( NULL );
                  dataSetMapper->UnRegister( NULL );
                  texture->UnRegister( NULL );

                  // Store the actor so that it may be accessed in following
                  // passes.
                  m_ImageActors[imageMapper].Initialize(imageActor, imageMapper, m_ImageMapperDeletedCommand);
                }
                else
                {
                  // Else, retrieve the actor and associated objects from the
                  // previous pass.
                  imageActor = m_ImageActors[imageMapper].m_Actor;
                  dataSetMapper = (vtkDataSetMapper *)imageActor->GetMapper();
                  texture = imageActor->GetTexture();

                  //BUG (#1551) added dynamic cast for VTK5.2 support
                  #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
                  lookupTable = dynamic_cast<vtkLookupTable*>(texture->GetLookupTable());
                  #else
                  lookupTable = texture->GetLookupTable();
                  #endif
                }

                // Set poly data new each time its object changes (e.g. when
                // switching between planar and curved geometries)
                if ( dataSetMapper != NULL )
                {
                  if ( dataSetMapper->GetInput() != surface->GetVtkPolyData() )
                  {
                    dataSetMapper->SetInput( surface->GetVtkPolyData() );
                  }
                }

                imageActor->GetMapper()->GetInput()->Update();
                imageActor->GetMapper()->Update();

                // We have to do this before GenerateAllData() is called
                // since there may be no RendererInfo for renderer yet,
                // thus GenerateAllData won't update the (non-existing)
                // RendererInfo for renderer. By calling GetRendererInfo
                // a RendererInfo will be created for renderer (if it does not
                // exist yet).
                imageMapper->GetRendererInfo( planeRenderer );

                imageMapper->GenerateAllData();

                // ensure the right openGL context, as 3D widgets may render and take their plane texture from 2D image mappers
                renderer->GetRenderWindow()->MakeCurrent();

                // Retrieve and update image to be mapped
                const ImageMapper2D::RendererInfo *rit =
                  imageMapper->GetRendererInfo( planeRenderer );
                if(rit->m_Image!=NULL)
                {
                  rit->m_Image->Update();

                  texture->SetInput( rit->m_Image );


                  // check for level-window-prop and use it if it exists
                  ScalarType windowMin = 0.0;
                  ScalarType windowMax = 255.0;
                  LevelWindow levelWindow;

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
                  bool useColor;
                  if ( !node->GetBoolProperty( "use color", useColor, planeRenderer ) )
                  {
                    useColor = false;
                  }

                  if ( binary )
                  {
                    useColor = true;
                  }

                  // check for LookupTable
                  LookupTableProperty::Pointer lookupTableProp;
                  lookupTableProp = dynamic_cast< LookupTableProperty * >(
                    node->GetPropertyList()
                      ->GetProperty( "LookupTable" ));

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
                    if ( lutProperties.LookupTableSource != NULL )
                    {
                      lutProperties.LookupTableSource->Delete();
                    }
                    lutProperties.LookupTableSource = lookupTableSource;
                    lutProperties.LookupTableSource->Register( NULL );
                    lutProperties.windowMin = windowMin;
                    lutProperties.windowMax = windowMax;

                    lookupTable->DeepCopy( lookupTableSource );
                    lookupTable->SetRange( windowMin, windowMax );
                  }

                  // Apply color property (of the node, not of the plane)
                  float rgb[3] = { 1.0, 1.0, 1.0 };
                  node->GetColor( rgb, renderer );
                  imageActor->GetProperty()->SetColor( rgb[0], rgb[1], rgb[2] );
                  //m_BackgroundActor->GetProperty()->SetColor(1,1,1);

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

    m_Cleaner->SetInput(surfacePolyData);

    m_EdgeTransformer->SetTransform(
      this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep()) );

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
    ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<ColorProperty*>(
      this->GetDataTreeNode()->GetProperty( "color" ));

    if ( colorProperty.IsNotNull() )
    {
      const Color& color = colorProperty->GetColor();
      m_EdgeActor->GetProperty()->SetColor(
        color.GetRed(), color.GetGreen(), color.GetBlue()
      );
    }
    else
    {
      m_EdgeActor->GetProperty()->SetColor( 1.0, 1.0, 1.0 );
    }

    m_ImageAssembly->SetUserTransform(
      this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep()) );
    }

  VtkRepresentationProperty* representationProperty;
  this->GetDataTreeNode()->GetProperty(representationProperty, "representation", renderer);
  if ( representationProperty != NULL )
    m_BackgroundActor->GetProperty()->SetRepresentation( representationProperty->GetVtkRepresentation() );

}

void
Geometry2DDataVtkMapper3D::ActorInfo::Initialize(vtkActor* actor, itk::Object* sender, itk::Command* command)
{
  m_Actor = actor;
  m_Sender = sender;
  // Get informed when ImageMapper object is deleted, so that
  // the data structures built here can be deleted as well
  m_ObserverID = sender->AddObserver( itk::DeleteEvent(), command );
}

Geometry2DDataVtkMapper3D::ActorInfo::ActorInfo() : m_Actor(NULL), m_Sender(NULL), m_ObserverID(0)
{
}

Geometry2DDataVtkMapper3D::ActorInfo::~ActorInfo()
{
  if(m_Sender != NULL)
  {
    m_Sender->RemoveObserver(m_ObserverID);
  }
  if(m_Actor != NULL)
  {
    m_Actor->Delete();
  }
}

} // namespace mitk
