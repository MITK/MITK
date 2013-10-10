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

#include "mitkLabelSetImageVtkMapper2D.h"

//MITK
#include <mitkDataNode.h>
#include <mitkDataNodeFactory.h>
#include <mitkLevelWindowProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkTimeSlicedGeometry.h>
#include <mitkColormapProperty.h>
#include <mitkPlaneClipping.h>
#include <mitkLabelSetImage.h>
#include <mitkRenderingModeProperty.h>
#include <vtkMitkLevelWindowFilter.h>

//VTK
#include <vtkProperty.h>
#include <vtkPlaneSource.h>
#include <vtkLookupTable.h>
//#include <vtkImageLabelOutline.h>
#include <vtkImageBlend.h>

mitk::LabelSetImageVtkMapper2D::LabelSetImageVtkMapper2D()
{
}

mitk::LabelSetImageVtkMapper2D::~LabelSetImageVtkMapper2D()
{
}

void mitk::LabelSetImageVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::DataNode* node = this->GetDataNode();

  mitk::LabelSetImage *image = dynamic_cast< mitk::LabelSetImage* >( node->GetData() );

  if ( image == NULL || image->IsInitialized() == false )
    return;

  //check if there is a valid worldGeometry
  const Geometry2D *worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( ( worldGeometry == NULL ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
    return;

  image->Update();

  // early out if there is no intersection of the current rendering geometry
  // and the geometry of the image that is to be rendered.
  if ( !RenderingGeometryIntersectsImage( worldGeometry, image->GetSlicedGeometry() ) )
  {
    // set image to NULL, to clear the texture in 3D, because
    // the latest image is used there if the plane is out of the geometry
    // see bug-13275
    localStorage->m_ReslicedImage = NULL;
    localStorage->m_Mapper->SetInput( localStorage->m_EmptyPolyData );
    localStorage->m_OutlineActor->SetVisibility(false);
    localStorage->m_OutlineShadowActor->SetVisibility(false);
    return;
  }

//  int numberOfLayers = image->GetNumberOfLayers();
  int activeLayer = image->GetActiveLayer();
  int activeLabel = image->GetActiveLabelIndex(activeLayer);

//  for (int layer=0; layer<numberOfLayers; ++layer)
//  {
//    LabelSetImage::LabelSetPixelType* layerBuffer = image->GetLayerBufferPointer(layer);
    //set main input for ExtractSliceFilter
  localStorage->m_Reslicer->SetInput(image);
  localStorage->m_Reslicer->SetWorldGeometry(worldGeometry);
  localStorage->m_Reslicer->SetTimeStep( this->GetTimestep() );

  //set the transformation of the image to adapt reslice axis
  localStorage->m_Reslicer->SetResliceTransformByGeometry( image->GetTimeSlicedGeometry()->GetGeometry3D( this->GetTimestep() ) );

  //is the geometry of the slice based on the image image or the worldgeometry?
  bool inPlaneResampleExtentByGeometry = false;
  node->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
  localStorage->m_Reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);

  localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);

  //set the vtk output property to true, makes sure that no unneeded mitk image convertion
  //is done.
  localStorage->m_Reslicer->SetVtkOutputRequest(true);

  //this is needed when thick mode was enable bevore. These variable have to be reset to default values
  localStorage->m_Reslicer->SetOutputDimensionality(2);
  localStorage->m_Reslicer->SetOutputSpacingZDirection(1.0);
  localStorage->m_Reslicer->SetOutputExtentZDirection(0,0);

  // Bounds information for reslicing (only reuqired if reference geometry
  // is present)
  //this used for generating a vtkPLaneSource with the right size
  double sliceBounds[6];
  for (int i=0; i<6; ++i)
  {
    sliceBounds[i] = 0.0;
  }
  localStorage->m_Reslicer->GetClippedPlaneBounds(sliceBounds);

  //get the spacing of the slice
  localStorage->m_mmPerPixel = localStorage->m_Reslicer->GetOutputSpacing();

  double textureClippingBounds[6];
  for (int i=0; i<6; ++i)
  {
    textureClippingBounds[i] = 0.0;
  }

  localStorage->m_Reslicer->Modified();
  //start the pipeline with updating the largest possible, needed if the geometry of the image has changed
  localStorage->m_Reslicer->UpdateLargestPossibleRegion();
  localStorage->m_ReslicedImage = localStorage->m_Reslicer->GetVtkOutput();

  //generate contours/outlines
  localStorage->m_OutlinePolyData = this->CreateOutlinePolyData( renderer, activeLabel );

//  if (layer == activeLayer)
//  {
    bool contourAll = false;
    node->GetBoolProperty( "labelset.contour.all", contourAll, renderer );

    bool contourActive = false;
    node->GetBoolProperty( "labelset.contour.active", contourActive, renderer );
/*
    if ( contourAll || contourActive )
    {
      int activeLayer = image->GetActiveLayer();
      localStorage->m_LabelOutline->SetInput( localStorage->m_ReslicedImage );
      localStorage->m_LabelOutline->SetActiveLabel( image->GetActiveLabelIndex(activeLayer) );
      localStorage->m_LabelOutline->SetOutlineAll( contourAll );
      localStorage->m_LabelOutline->SetBackground(0.0);
      localStorage->m_LabelOutline->Update();
      localStorage->m_ReslicedImage = localStorage->m_LabelOutline->GetOutput();
    }
*/
//  }

//    localStorage->m_ImageBlend->SetInput(layer, localStorage->m_ReslicedImage);
//    localStorage->m_ImageBlend->SetOpacity(layer, 1-(double)layer/(double)numberOfLayers);
// }

//  localStorage->m_ImageBlend->Update();

  const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( worldGeometry );

  // Calculate the actual bounds of the transformed plane clipped by the
  // dataset bounding box; this is required for drawing the texture at the
  // correct position during 3D mapping.
  mitk::PlaneClipping::CalculateClippedPlaneBounds( image->GetGeometry(), planeGeometry, textureClippingBounds );

  textureClippingBounds[0] = static_cast< int >( textureClippingBounds[0] / localStorage->m_mmPerPixel[0] + 0.5 );
  textureClippingBounds[1] = static_cast< int >( textureClippingBounds[1] / localStorage->m_mmPerPixel[0] + 0.5 );
  textureClippingBounds[2] = static_cast< int >( textureClippingBounds[2] / localStorage->m_mmPerPixel[1] + 0.5 );
  textureClippingBounds[3] = static_cast< int >( textureClippingBounds[3] / localStorage->m_mmPerPixel[1] + 0.5 );

  //clipping bounds for cutting the image
  localStorage->m_LevelWindowFilter->SetClippingBounds(textureClippingBounds);

  float contourWidth(2.0);
  node->GetFloatProperty( "labelset.contour.width", contourWidth, renderer );

  localStorage->m_OutlineActor->GetProperty()->SetLineWidth( contourWidth );
  localStorage->m_OutlineShadowActor->GetProperty()->SetLineWidth( contourWidth * 1.5 );

  localStorage->m_OutlineActor->SetVisibility(true);
  localStorage->m_OutlineShadowActor->SetVisibility(true);

  this->ApplyColor( renderer );
  this->ApplyOpacity( renderer );
  this->ApplyLookuptable( renderer );

  // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
  localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

  //connect the image with the levelwindow filter
  localStorage->m_LevelWindowFilter->SetInput( localStorage->m_ReslicedImage ); //localStorage->m_ImageBlend->GetOutput() );
  //connect the texture with the output of the levelwindow filter

  // check for texture interpolation property
  bool textureInterpolation = false;
  node->GetBoolProperty( "texture interpolation", textureInterpolation, renderer );

  //set the interpolation modus according to the property
  localStorage->m_Texture->SetInterpolate(textureInterpolation);

  localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowFilter->GetOutputPort());

  this->TransformActor( renderer );

  //we need the contour for the binary outline property as actor
  localStorage->m_OutlineMapper->SetInput( localStorage->m_OutlinePolyData );
//  localStorage->m_OutlineActor->SetTexture(NULL); //no texture for contours

  //setup the textured plane
  this->GeneratePlane( renderer, sliceBounds );
  //set the plane as input for the mapper
  localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
  //set the texture for the actor

  localStorage->m_Actor->SetTexture(localStorage->m_Texture);
}

void mitk::LabelSetImageVtkMapper2D::ApplyColor( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  mitk::LabelSetImage *image = dynamic_cast<mitk::LabelSetImage*>(this->GetDataNode()->GetData());
  assert(image);
  int activeLayer = image->GetActiveLayer();
  const mitk::Color& activeLabelColor = image->GetActiveLabelColor( activeLayer );
  double rgbConv[3] = {(double)activeLabelColor.GetRed(), (double)activeLabelColor.GetGreen(), (double)activeLabelColor.GetBlue()};
  localStorage->m_OutlineActor->GetProperty()->SetColor(rgbConv);
  rgbConv[0] = 0.0; rgbConv[1] = 0.0; rgbConv[2] = 0.0;
  localStorage->m_OutlineShadowActor->GetProperty()->SetColor(rgbConv);
}

void mitk::LabelSetImageVtkMapper2D::ApplyOpacity( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity( opacity, renderer, "opacity" );
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  localStorage->m_OutlineActor->GetProperty()->SetOpacity(opacity);
  localStorage->m_OutlineShadowActor->GetProperty()->SetOpacity(opacity);
}

void mitk::LabelSetImageVtkMapper2D::ApplyLookuptable( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::LabelSetImage* input = dynamic_cast<mitk::LabelSetImage*>(this->GetDataNode()->GetData());
  localStorage->m_LevelWindowFilter->SetLookupTable( input->GetLookupTable(input->GetActiveLayer())->GetVtkLookupTable() );
}

void mitk::LabelSetImageVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  const DataNode *node = this->GetDataNode();
  node->GetVisibility(visible, renderer, "visible");

  if ( !visible )
    return;

  mitk::LabelSetImage *image = dynamic_cast< mitk::LabelSetImage* >( node->GetData() );

  if ( image == NULL || image->IsInitialized() == false )
    return;

  // Calculate time step of the image data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const TimeSlicedGeometry *dataTimeGeometry = image->GetTimeSlicedGeometry();
  if ( ( dataTimeGeometry == NULL )
       || ( dataTimeGeometry->GetTimeSteps() == 0 )
       || ( !dataTimeGeometry->IsValidTime( this->GetTimestep() ) ) )
  {
    return;
  }

  image->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender

//(localStorage->m_LastDataUpdateTime < node->GetMTime()) // this one is too generic
  if ( (localStorage->m_LastDataUpdateTime < image->GetMTime()) //was the data modified?
       || (localStorage->m_LastDataUpdateTime < image->GetPipelineMTime())
       || (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) //was the geometry modified?
       || (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime())
      )
  {
    this->GenerateDataForRenderer( renderer );
    localStorage->m_LastDataUpdateTime.Modified();
  }
  else if ( (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
       || (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->ApplyColor(renderer);
    this->ApplyOpacity(renderer);
    this->ApplyLookuptable( renderer );
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
}

void mitk::LabelSetImageVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::LabelSetImage* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image);

  // add/replace the following properties
  node->SetProperty( "opacity", FloatProperty::New(1.0f), renderer );
  node->SetProperty( "color", ColorProperty::New(1.0,1.0,1.0), renderer );
  node->SetProperty( "binary", BoolProperty::New( false ), renderer );

  mitk::RenderingModeProperty::Pointer renderingModeProperty = mitk::RenderingModeProperty::New( RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW );
  node->SetProperty( "Image Rendering.Mode", renderingModeProperty, renderer );

  mitk::ColormapProperty::Pointer colormapProperty = mitk::ColormapProperty::New(ColormapProperty::CM_MULTILABEL);
  node->SetProperty( "colormap", colormapProperty, renderer );

  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  levelwindow.SetLevelWindow(127.5, 255);
  levelwindow.SetRangeMinMax(0, 255);
  levWinProp->SetLevelWindow(levelwindow);
  node->SetProperty( "levelwindow", levWinProp, renderer);

  node->SetProperty( "labelset.contour.all", BoolProperty::New( false ), renderer );
  node->SetProperty( "labelset.contour.active", BoolProperty::New( true ), renderer );
  node->SetProperty( "labelset.contour.width", FloatProperty::New( 2.0 ), renderer );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
