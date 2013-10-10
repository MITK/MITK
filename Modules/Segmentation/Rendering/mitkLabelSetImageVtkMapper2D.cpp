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
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkPlaneSource.h>
#include <vtkCellArray.h>
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

  mitk::LabelSetImage *input = dynamic_cast< mitk::LabelSetImage* >( node->GetData() );

  if ( input == NULL || input->IsInitialized() == false )
    return;

  //check if there is a valid worldGeometry
  const Geometry2D *worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( ( worldGeometry == NULL ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
    return;

  input->Update();

  // early out if there is no intersection of the current rendering geometry
  // and the geometry of the image that is to be rendered.
  if ( !RenderingGeometryIntersectsImage( worldGeometry, input->GetSlicedGeometry() ) )
  {
    // set image to NULL, to clear the texture in 3D, because
    // the latest image is used there if the plane is out of the geometry
    // see bug-13275
    localStorage->m_ReslicedImage = NULL;
    localStorage->m_Mapper->SetInput( localStorage->m_EmptyPolyData );
    localStorage->m_OutlineActor->SetVisibility(false);
    return;
  }

//  int numberOfLayers = input->GetNumberOfLayers();
//  int activeLayer = input->GetActiveLayer();

//  for (int layer=0; layer<numberOfLayers; ++layer)
//  {
//    LabelSetImage::LabelSetPixelType* layerBuffer = input->GetLayerBufferPointer(layer);
    //set main input for ExtractSliceFilter
  localStorage->m_Reslicer->SetInput(input);
  localStorage->m_Reslicer->SetWorldGeometry(worldGeometry);
  localStorage->m_Reslicer->SetTimeStep( this->GetTimestep() );

  //set the transformation of the image to adapt reslice axis
  localStorage->m_Reslicer->SetResliceTransformByGeometry( input->GetTimeSlicedGeometry()->GetGeometry3D( this->GetTimestep() ) );

  //is the geometry of the slice based on the input image or the worldgeometry?
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
  //start the pipeline with updating the largest possible, needed if the geometry of the input has changed
  localStorage->m_Reslicer->UpdateLargestPossibleRegion();
  localStorage->m_ReslicedImage = localStorage->m_Reslicer->GetVtkOutput();

  //generate contours/outlines
  localStorage->m_OutlinePolyData = this->CreateOutlinePolyData( renderer );

//  if (layer == activeLayer)
//  {
    bool contourAll = false;
    node->GetBoolProperty( "labelset.contour.all", contourAll, renderer );

    bool contourActive = false;
    node->GetBoolProperty( "labelset.contour.active", contourActive, renderer );
/*
    if ( contourAll || contourActive )
    {
      int activeLayer = input->GetActiveLayer();
      localStorage->m_LabelOutline->SetInput( localStorage->m_ReslicedImage );
      localStorage->m_LabelOutline->SetActiveLabel( input->GetActiveLabelIndex(activeLayer) );
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
  mitk::PlaneClipping::CalculateClippedPlaneBounds( input->GetGeometry(), planeGeometry, textureClippingBounds );

  textureClippingBounds[0] = static_cast< int >( textureClippingBounds[0] / localStorage->m_mmPerPixel[0] + 0.5 );
  textureClippingBounds[1] = static_cast< int >( textureClippingBounds[1] / localStorage->m_mmPerPixel[0] + 0.5 );
  textureClippingBounds[2] = static_cast< int >( textureClippingBounds[2] / localStorage->m_mmPerPixel[1] + 0.5 );
  textureClippingBounds[3] = static_cast< int >( textureClippingBounds[3] / localStorage->m_mmPerPixel[1] + 0.5 );

  //clipping bounds for cutting the image
  localStorage->m_LevelWindowFilter->SetClippingBounds(textureClippingBounds);

  float contourWidth = 2.5;
  node->GetFloatProperty( "labelset.contour.width", contourWidth, renderer );
  localStorage->m_OutlineActor->GetProperty()->SetLineWidth( contourWidth );
  localStorage->m_OutlineActor->SetVisibility(true);

  this->ApplyColor( renderer );
  this->ApplyOpacity( renderer );
  this->ApplyLookuptable( renderer );

  // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
  localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

  //connect the input with the levelwindow filter
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
  mitk::LabelSetImage *input = dynamic_cast<mitk::LabelSetImage*>(this->GetDataNode()->GetData());
  int activeLayer = input->GetActiveLayer();
  const mitk::Color& activeLabelColor = input->GetActiveLabelColor( activeLayer );
  double rgbConv[3] = {(double)activeLabelColor.GetRed(), (double)activeLabelColor.GetGreen(), (double)activeLabelColor.GetBlue()};
  localStorage->m_OutlineActor->GetProperty()->SetColor(rgbConv);
}

void mitk::LabelSetImageVtkMapper2D::ApplyOpacity( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity( opacity, renderer, "opacity" );
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  localStorage->m_OutlineActor->GetProperty()->SetOpacity(opacity);
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

/*
  // Properties common for both images and segmentations
  node->AddProperty( "depthOffset", FloatProperty::New( 0.0 ), renderer, overwrite );
  if(image->IsRotated()) node->AddProperty( "reslice interpolation", VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC) );
  else node->AddProperty( "reslice interpolation", VtkResliceInterpolationProperty::New() );
  node->AddProperty( "texture interpolation", BoolProperty::New( mitk::DataNodeFactory::m_TextureInterpolationActive ) );  // set to user configurable default value (see global options)
  node->AddProperty( "in plane resample extent by geometry", BoolProperty::New( false ) );
  node->AddProperty( "bounding box", BoolProperty::New( false ) );
*/
  mitk::RenderingModeProperty::Pointer renderingModeProperty = mitk::RenderingModeProperty::New( RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW );
  node->AddProperty( "Image Rendering.Mode", renderingModeProperty);

  mitk::ColormapProperty::Pointer colormapProperty = mitk::ColormapProperty::New();
  node->AddProperty( "colormap", colormapProperty, renderer, overwrite );
  colormapProperty->SetValue( ColormapProperty::CM_MULTILABEL );

  node->AddProperty( "opacity", FloatProperty::New(0.9f), renderer, overwrite );
  node->AddProperty( "color", ColorProperty::New(1.0,1.0,1.0), renderer, overwrite );
  node->AddProperty( "binary", BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "labelset.contour.all", BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "labelset.contour.active", BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "labelset.contour.width", FloatProperty::New( 2.5 ), renderer, overwrite );

  node->AddProperty( "layer", IntProperty::New(100), renderer, overwrite);

  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  levelwindow.SetLevelWindow(127.5, 255.0);
  levelwindow.SetRangeMinMax(0, 255);
  levWinProp->SetLevelWindow( levelwindow );
  node->AddProperty( "levelwindow", levWinProp, renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkSmartPointer<vtkPolyData> mitk::LabelSetImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage(renderer);

  const DataNode *node = this->GetDataNode();
  mitk::LabelSetImage *image = dynamic_cast< mitk::LabelSetImage* >( node->GetData() );

  if ( image == NULL || image->IsInitialized() == false )
    return NULL;

  //get the min and max index values of each direction
  int* extent = localStorage->m_ReslicedImage->GetExtent();
  int xMin = extent[0];
  int xMax = extent[1];
  int yMin = extent[2];
  int yMax = extent[3];

  int* dims = localStorage->m_ReslicedImage->GetDimensions(); //dimensions of the image
  int line = dims[0]; //how many pixels per line?
  int x = xMin; //pixel index x
  int y = yMin; //pixel index y
  LabelSetImage::PixelType* currentPixel;

  //get the depth for each contour
  float depth = this->CalculateLayerDepth(renderer);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); //the lines to connect the points

  // We take the pointer to the first pixel of the image
  currentPixel = static_cast< LabelSetImage::PixelType* >( localStorage->m_ReslicedImage->GetScalarPointer() );

  int activeLayer = image->GetActiveLayer();
  int activeLabel = image->GetActiveLabelIndex(activeLayer);

  while (y <= yMax)
  {
    //if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel == activeLabel))
    {
      //check in which direction a line is necessary
      //a line is added if the neighbor of the current pixel has the value 0
      //and if the pixel is located at the edge of the image

      //if   vvvvv  not the first line vvvvv
      if (y > yMin && *(currentPixel-line) != activeLabel)
      { //x direction - bottom edge of the pixel
        //add the 2 points
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        //add the line between both points
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last line vvvvv
      if (y < yMax && *(currentPixel+line) != activeLabel)
      { //x direction - top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the first pixel vvvvv
      if ( (x > xMin || y > yMin) && *(currentPixel-1) != activeLabel)
      { //y direction - left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last pixel vvvvv
      if ( (y < yMax || (x < xMax) ) && *(currentPixel+1) != activeLabel)
      { //y direction - right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      /*  now consider pixels at the edge of the image  */

      //if   vvvvv  left edge of image vvvvv
      if (x == xMin)
      { //draw left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  right edge of image vvvvv
      if (x == xMax)
      { //draw right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  bottom edge of image vvvvv
      if (y == yMin)
      { //draw bottom edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  top edge of image vvvvv
      if (y == yMax)
      { //draw top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
    }//end if currentpixel is set

    x++;

    if (x > xMax)
    { //reached end of line
      x = xMin;
      y++;
    }

    // Increase the pointer-position to the next pixel.
    // This is safe, as the while-loop and the x-reset logic above makes
    // sure we do not exceed the bounds of the image
    currentPixel++;
  }//end of while

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  return polyData;
}

void mitk::LabelSetImageVtkMapper2D::TransformActor(mitk::BaseRenderer* renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  //get the transformation matrix of the reslicer in order to render the slice as axial, coronal or saggital
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_Reslicer->GetResliceAxes();
  trans->SetMatrix(matrix);
  //transform the plane/contour (the actual actor) to the corresponding view (axial, coronal or saggital)
  localStorage->m_Actor->SetUserTransform(trans);
  //transform the origin to center based coordinates, because MITK is center based.
  localStorage->m_Actor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);
  localStorage->m_OutlineActor->SetUserTransform(trans);
  localStorage->m_OutlineActor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);
}
