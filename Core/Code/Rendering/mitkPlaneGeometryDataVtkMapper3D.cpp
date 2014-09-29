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

#include "mitkPlaneGeometryDataVtkMapper3D.h"

#include "mitkImageVtkMapper2D.h"
#include "mitkSmartPointerProperty.h"
#include "mitkSurface.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkWeakPointerProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateOr.h"
#include "vtkNeverTranslucentTexture.h"
#include "vtkMitkLevelWindowFilter.h"

#include <vtkAssembly.h>
#include <vtkDataSetMapper.h>
#include <vtkFeatureEdges.h>
#include <vtkHedgeHog.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>

namespace mitk
{

  PlaneGeometryDataVtkMapper3D::PlaneGeometryDataVtkMapper3D()
    : m_NormalsActorAdded(false),
    m_DataStorage(NULL)
  {
    m_EdgeTuber = vtkTubeFilter::New();
    m_EdgeMapper = vtkPolyDataMapper::New();

    m_SurfaceCreator = PlaneGeometryDataToSurfaceFilter::New();
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
    m_Cleaner->SetInputData( emptyPolyData );
    emptyPolyData->Delete();

    m_Edges->SetInputConnection(m_Cleaner->GetOutputPort());
    m_EdgeTransformer->SetInputConnection( m_Edges->GetOutputPort() );

    m_EdgeTuber->SetInputConnection( m_EdgeTransformer->GetOutputPort() );
    m_EdgeTuber->SetVaryRadiusToVaryRadiusOff();
    m_EdgeTuber->SetNumberOfSides( 12 );
    m_EdgeTuber->CappingOn();

    m_EdgeMapper->SetInputConnection( m_EdgeTuber->GetOutputPort() );
    m_EdgeMapper->ScalarVisibilityOff();

    m_BackgroundMapper->SetInputData(emptyPolyData);
    m_BackgroundMapper->Update();

    m_EdgeActor->SetMapper( m_EdgeMapper );

    m_BackgroundActor->GetProperty()->SetAmbient( 0.5 );
    m_BackgroundActor->GetProperty()->SetColor( 0.0, 0.0, 0.0 );
    m_BackgroundActor->GetProperty()->SetOpacity( 0.0 );
    m_BackgroundActor->SetMapper( m_BackgroundMapper );

    vtkProperty * backfaceProperty = m_BackgroundActor->MakeProperty();
    backfaceProperty->SetColor( 0.0, 0.0, 0.0 );
    m_BackgroundActor->SetBackfaceProperty( backfaceProperty );
    backfaceProperty->Delete();

    m_FrontHedgeHog = vtkHedgeHog::New();
    m_BackHedgeHog  = vtkHedgeHog::New();

    m_FrontNormalsMapper = vtkPolyDataMapper::New();
    m_FrontNormalsMapper->SetInputConnection( m_FrontHedgeHog->GetOutputPort());
    m_BackNormalsMapper = vtkPolyDataMapper::New();

    m_Prop3DAssembly->AddPart( m_EdgeActor );
    m_Prop3DAssembly->AddPart( m_ImageAssembly );
    m_FrontNormalsActor = vtkActor::New();
    m_FrontNormalsActor->SetMapper(m_FrontNormalsMapper);
    m_BackNormalsActor = vtkActor::New();
    m_BackNormalsActor->SetMapper(m_BackNormalsMapper);

    m_ImageMapperDeletedCommand = MemberCommandType::New();
    m_ImageMapperDeletedCommand->SetCallbackFunction(
        this, &PlaneGeometryDataVtkMapper3D::ImageMapperDeletedCallback );
  }

  PlaneGeometryDataVtkMapper3D::~PlaneGeometryDataVtkMapper3D()
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
    m_FrontNormalsMapper->Delete();
    m_FrontNormalsActor->Delete();
    m_FrontHedgeHog->Delete();
    m_BackNormalsMapper->Delete();
    m_BackNormalsActor->Delete();
    m_BackHedgeHog->Delete();

    for(ActorList::iterator it = m_ImageActors.begin();
        it!=m_ImageActors.end(); ++it)
      it->second.m_Actor->ReleaseGraphicsResources(0);

    // Delete entries in m_ImageActors list one by one
    m_ImageActors.clear();

    m_DataStorage = NULL;
  }

  vtkProp* PlaneGeometryDataVtkMapper3D::GetVtkProp(mitk::BaseRenderer * /*renderer*/)
  {
    if ( (this->GetDataNode() != NULL )
      && (m_ImageAssembly != NULL) )
      {
      // Do not transform the entire Prop3D assembly, but only the image part
      // here. The colored frame is transformed elsewhere (via m_EdgeTransformer),
      // since only vertices should be transformed there, not the poly data
      // itself, to avoid distortion for anisotropic datasets.
      m_ImageAssembly->SetUserTransform( this->GetDataNode()->GetVtkTransform() );
    }
    return m_Prop3DAssembly;
  }

  void PlaneGeometryDataVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
  {
    m_ImageAssembly->SetUserTransform(
        this->GetDataNode()->GetVtkTransform(this->GetTimestep()) );
  }

  const PlaneGeometryData* PlaneGeometryDataVtkMapper3D::GetInput()
  {
    return static_cast<const PlaneGeometryData * > ( GetDataNode()->GetData() );
  }

  void PlaneGeometryDataVtkMapper3D::SetDataStorageForTexture(mitk::DataStorage* storage)
  {
    if(storage != NULL && m_DataStorage != storage )
    {
      m_DataStorage = storage;
      this->Modified();
    }
  }

  void PlaneGeometryDataVtkMapper3D::ImageMapperDeletedCallback(
      itk::Object *caller, const itk::EventObject& /*event*/ )
  {
    ImageVtkMapper2D *imageMapper = dynamic_cast< ImageVtkMapper2D * >( caller );
    if ( (imageMapper != NULL) )
    {
      if ( m_ImageActors.count( imageMapper ) > 0)
      {
        m_ImageActors[imageMapper].m_Sender = NULL; // sender is already destroying itself
        m_ImageActors.erase( imageMapper );
      }
    }
  }

  void PlaneGeometryDataVtkMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
  {
    SetVtkMapperImmediateModeRendering(m_EdgeMapper);
    SetVtkMapperImmediateModeRendering(m_BackgroundMapper);

    // Remove all actors from the assembly, and re-initialize it with the
    // edge actor
    m_ImageAssembly->GetParts()->RemoveAllItems();

    bool visible = true;
    GetDataNode()->GetVisibility(visible, renderer, "visible");

    if ( !visible )
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
    bool drawEdges = true;
    this->GetDataNode()->GetBoolProperty("draw edges", drawEdges, renderer);
    m_EdgeActor->SetVisibility(drawEdges);

    PlaneGeometryData::Pointer input = const_cast< PlaneGeometryData * >(this->GetInput());

    if (input.IsNotNull() && (input->GetPlaneGeometry() != NULL))
    {
      SmartPointerProperty::Pointer surfacecreatorprop;
      surfacecreatorprop = dynamic_cast< SmartPointerProperty * >(GetDataNode()->GetProperty("surfacegeometry", renderer));

      if ( (surfacecreatorprop.IsNull())
        || (surfacecreatorprop->GetSmartPointer().IsNull())
        || ((m_SurfaceCreator = dynamic_cast<PlaneGeometryDataToSurfaceFilter*>
             (surfacecreatorprop->GetSmartPointer().GetPointer())).IsNull() ) )
        {
        m_SurfaceCreator->PlaceByGeometryOn();
        surfacecreatorprop = SmartPointerProperty::New( m_SurfaceCreator );
        GetDataNode()->SetProperty("surfacegeometry", surfacecreatorprop);
      }

      m_SurfaceCreator->SetInput(input);

      int res;
      if (GetDataNode()->GetIntProperty("xresolution", res, renderer))
      {
        m_SurfaceCreator->SetXResolution(res);
      }
      if (GetDataNode()->GetIntProperty("yresolution", res, renderer))
      {
        m_SurfaceCreator->SetYResolution(res);
      }

      double tubeRadius = 1.0; // Radius of tubular edge surrounding plane

      // Clip the PlaneGeometry with the reference geometry bounds (if available)
      if ( input->GetPlaneGeometry()->HasReferenceGeometry() )
      {
        BaseGeometry *referenceGeometry =
            input->GetPlaneGeometry()->GetReferenceGeometry();

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

        tubeRadius = referenceGeometry->GetDiagonalLength() / 450.0;
      }

      // If no reference geometry is available, clip with the current global
      // bounds
      else if (m_DataStorage.IsNotNull())
      {
        m_SurfaceCreator->SetBoundingBox(m_DataStorage->ComputeVisibleBoundingBox(NULL, "includeInBoundingBox"));
        tubeRadius = sqrt( m_SurfaceCreator->GetBoundingBox()->GetDiagonalLength2() ) / 450.0;
      }

      // Calculate the surface of the PlaneGeometry
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
      DataNode* node = this->GetDataNode();
      bool displayNormals = false;
      bool colorTwoSides = false;
      bool invertNormals = false;
      node->GetBoolProperty("draw normals 3D", displayNormals, renderer);
      node->GetBoolProperty("color two sides", colorTwoSides, renderer);
      node->GetBoolProperty("invert normals", invertNormals, renderer);

      //if we want to draw the display normals or render two sides we have to get the colors
      if( displayNormals || colorTwoSides )
      {
        //get colors
        float frontColor[3] = { 0.0, 0.0, 1.0 };
        node->GetColor( frontColor, renderer, "front color" );
        float backColor[3] = { 1.0, 0.0, 0.0 };
        node->GetColor( backColor, renderer, "back color" );

        if ( displayNormals )
        {
          m_NormalsTransformer->SetInputData( surface->GetVtkPolyData() );
          m_NormalsTransformer->SetTransform(node->GetVtkTransform(this->GetTimestep()) );

          m_FrontHedgeHog->SetInputConnection(m_NormalsTransformer->GetOutputPort() );
          m_FrontHedgeHog->SetVectorModeToUseNormal();
          m_FrontHedgeHog->SetScaleFactor( invertNormals ? 1.0 : -1.0 );
          m_FrontHedgeHog->Update();

          m_FrontNormalsActor->GetProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );

          m_BackHedgeHog->SetInputConnection( m_NormalsTransformer->GetOutputPort() );
          m_BackHedgeHog->SetVectorModeToUseNormal();
          m_BackHedgeHog->SetScaleFactor( invertNormals ? -1.0 : 1.0 );
          m_BackHedgeHog->Update();

          m_BackNormalsActor->GetProperty()->SetColor( backColor[0], backColor[1], backColor[2] );

          //if there is no actor added yet, add one
          if ( !m_NormalsActorAdded )
          {
            m_Prop3DAssembly->AddPart( m_FrontNormalsActor );
            m_Prop3DAssembly->AddPart( m_BackNormalsActor );
            m_NormalsActorAdded = true;
          }
        }
        //if we don't want to display normals AND there is an actor added remove the actor
        else if ( m_NormalsActorAdded )
        {
          m_Prop3DAssembly->RemovePart( m_FrontNormalsActor );
          m_Prop3DAssembly->RemovePart( m_BackNormalsActor );
          m_NormalsActorAdded = false;
        }

        if ( colorTwoSides )
        {
          if ( !invertNormals )
          {
            m_BackgroundActor->GetProperty()->SetColor( backColor[0], backColor[1], backColor[2] );
            m_BackgroundActor->GetBackfaceProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );
          }
          else
          {
            m_BackgroundActor->GetProperty()->SetColor( frontColor[0], frontColor[1], frontColor[2] );
            m_BackgroundActor->GetBackfaceProperty()->SetColor( backColor[0], backColor[1], backColor[2] );
          }
        }
      }

      // Add black background for all images (which may be transparent)
      m_BackgroundMapper->SetInputData( surface->GetVtkPolyData() );
      m_ImageAssembly->AddPart( m_BackgroundActor );

      LayerSortedActorList layerSortedActors;

      // Traverse the data tree to find nodes resliced by ImageMapperGL2D
      //use a predicate to get all data nodes which are "images" or inherit from mitk::Image
      mitk::TNodePredicateDataType< mitk::Image >::Pointer predicateAllImages = mitk::TNodePredicateDataType< mitk::Image >::New();
      mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetSubset(predicateAllImages);
      //process all found images
      for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
      {

        DataNode *node = it->Value();
        if (node != NULL)
          this->ProcessNode(node, renderer, surface, layerSortedActors);
      }

      // Add all image actors to the assembly, sorted according to
      // layer property
      LayerSortedActorList::iterator actorIt;
      for ( actorIt = layerSortedActors.begin(); actorIt != layerSortedActors.end(); ++actorIt )
      {
        m_ImageAssembly->AddPart( actorIt->second );
      }

      // Configurate the tube-shaped frame: size according to the surface
      // bounds, color as specified in the plane's properties
      vtkPolyData *surfacePolyData = surface->GetVtkPolyData();
      m_Cleaner->SetInputData(surfacePolyData);
      m_EdgeTransformer->SetTransform(this->GetDataNode()->GetVtkTransform(this->GetTimestep()) );

      // Adjust the radius according to extent
      m_EdgeTuber->SetRadius( tubeRadius );

      // Get the plane's color and set the tube properties accordingly
      ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<ColorProperty*>(this->GetDataNode()->GetProperty( "color" ));
      if ( colorProperty.IsNotNull() )
      {
        const Color& color = colorProperty->GetColor();
        m_EdgeActor->GetProperty()->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
      }
      else
      {
        m_EdgeActor->GetProperty()->SetColor( 1.0, 1.0, 1.0 );
      }

      m_ImageAssembly->SetUserTransform(this->GetDataNode()->GetVtkTransform(this->GetTimestep()) );
    }

    VtkRepresentationProperty* representationProperty;
    this->GetDataNode()->GetProperty(representationProperty, "material.representation", renderer);
    if ( representationProperty != NULL )
      m_BackgroundActor->GetProperty()->SetRepresentation( representationProperty->GetVtkRepresentation() );
  }

  void PlaneGeometryDataVtkMapper3D::ProcessNode( DataNode * node, BaseRenderer* renderer,
                                               Surface * surface, LayerSortedActorList &layerSortedActors )
  {
    if ( node != NULL )
    {
      //we need to get the information from the 2D mapper to render the texture on the 3D plane
      ImageVtkMapper2D *imageMapper = dynamic_cast< ImageVtkMapper2D * >( node->GetMapper(1) ); //GetMapper(1) provides the 2D mapper for the data node

      //if there is a 2D mapper, which is not the standard image mapper...
      if(!imageMapper && node->GetMapper(1))
      { //... check if it is the composite mapper
        std::string cname(node->GetMapper(1)->GetNameOfClass());
        if(!cname.compare("CompositeMapper")) //string.compare returns 0 if the two strings are equal.
        {
          //get the standard image mapper.
          //This is a special case in MITK and does only work for the CompositeMapper.
          imageMapper = dynamic_cast<ImageVtkMapper2D* >( node->GetMapper(3) );
        }
      }

      if ( (node->IsVisible(renderer)) && imageMapper )
      {
        WeakPointerProperty::Pointer rendererProp =
            dynamic_cast< WeakPointerProperty * >(GetDataNode()->GetPropertyList()->GetProperty("renderer"));

        if ( rendererProp.IsNotNull() )
        {
          BaseRenderer::Pointer planeRenderer = dynamic_cast< BaseRenderer * >(rendererProp->GetWeakPointer().GetPointer());
          // Retrieve and update image to be mapped
          const ImageVtkMapper2D::LocalStorage* localStorage = imageMapper->GetLocalStorage(planeRenderer);

          if ( planeRenderer.IsNotNull() )
          {
            // perform update of imagemapper if needed (maybe the respective 2D renderwindow is not rendered/update before)
            imageMapper->Update(planeRenderer);

            // If it has not been initialized already in a previous pass,
            // generate an actor and a texture object to
            // render the image associated with the ImageVtkMapper2D.
            vtkActor *imageActor;
            vtkDataSetMapper *dataSetMapper = NULL;
            vtkTexture *texture;
            if ( m_ImageActors.count( imageMapper ) == 0 )
            {
              dataSetMapper = vtkDataSetMapper::New();
              //Enable rendering without copying the image.
              dataSetMapper->ImmediateModeRenderingOn();

              texture = vtkNeverTranslucentTexture::New();
              texture->RepeatOff();

              imageActor = vtkActor::New();
              imageActor->SetMapper( dataSetMapper );
              imageActor->SetTexture( texture );
              imageActor->GetProperty()->SetOpacity(0.999); // HACK! otherwise VTK wouldn't recognize this as translucent surface (if LUT values map to alpha < 255
              // improvement: apply "opacity" property onle HERE and also in 2D image mapper. DO NOT change LUT to achieve translucent images (see method ChangeOpacity in image mapper 2D)

              // Make imageActor the sole owner of the mapper and texture
              // objects
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
            }

            // Set poly data new each time its object changes (e.g. when
            // switching between planar and curved geometries)
            if ( (dataSetMapper != NULL) && (dataSetMapper->GetInput() != surface->GetVtkPolyData()) )
            {
              dataSetMapper->SetInputData( surface->GetVtkPolyData() );
            }

            dataSetMapper->Update();

            //Check if the m_ReslicedImage is NULL.
            //This is the case when no image geometry is met by
            //the reslicer. In that case, the texture has to be
            //empty (black) and we don't have to do anything.
            //See fixed bug #13275
            if(localStorage->m_ReslicedImage != NULL)
            {
              texture->SetInputConnection(localStorage->m_LevelWindowFilter->GetOutputPort());

              // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
              texture->MapColorScalarsThroughLookupTableOff();

              //re-use properties from the 2D image mapper
              imageActor->SetProperty( localStorage->m_Actor->GetProperty() );
              imageActor->GetProperty()->SetAmbient(0.5);

              // Set texture interpolation on/off
              bool textureInterpolation = node->IsOn( "texture interpolation", renderer );
              texture->SetInterpolate( textureInterpolation );

              // Store this actor to be added to the actor assembly, sort
              // by layer
              int layer = 1;
              node->GetIntProperty( "layer", layer );
              layerSortedActors.insert(std::pair< int, vtkActor * >( layer, imageActor ) );
            }
          }
        }
      }
    }
  }

  void PlaneGeometryDataVtkMapper3D::ActorInfo::Initialize(vtkActor* actor, itk::Object* sender, itk::Command* command)
  {
    m_Actor = actor;
    m_Sender = sender;
    // Get informed when ImageMapper object is deleted, so that
    // the data structures built here can be deleted as well
    m_ObserverID = sender->AddObserver( itk::DeleteEvent(), command );
  }

  PlaneGeometryDataVtkMapper3D::ActorInfo::ActorInfo() : m_Actor(NULL), m_Sender(NULL), m_ObserverID(0)
  {
  }

  PlaneGeometryDataVtkMapper3D::ActorInfo::~ActorInfo()
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
