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
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkDataNodeFactory.h>
#include <mitkImageSliceSelector.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkTimeSlicedGeometry.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkPixelType.h>
#include <mitkColormapProperty.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkPlaneClipping.h>
#include <mitkLabelSetImage.h>

//MITK Rendering
#include "vtkMitkLevelWindowFilter.h"
#include "vtkNeverTranslucentTexture.h"
#include <mitkRenderingModeProperty.h>

//VTK
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkImageReslice.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
//#include <vtkImageLabelOutline.h>
#include <vtkImageBlend.h>

mitk::LabelSetImageVtkMapper2D::LabelSetImageVtkMapper2D()
{
}

mitk::LabelSetImageVtkMapper2D::~LabelSetImageVtkMapper2D()
{
  //The 3D RW Mapper (Geometry2DDataVtkMapper3D) is listening to this event,
  //in order to delete the images from the 3D RW.
  this->InvokeEvent( itk::DeleteEvent() );
}

//set the two points defining the textured plane according to the dimension and spacing
void mitk::LabelSetImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6])
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float depth = this->CalculateLayerDepth(renderer);
  //Set the origin to (xMin; yMin; depth) of the plane. This is necessary for obtaining the correct
  //plane size in crosshair rotation and swivel mode.
  localStorage->m_Plane->SetOrigin(planeBounds[0], planeBounds[2], depth);
  //These two points define the axes of the plane in combination with the origin.
  //Point 1 is the x-axis and point 2 the y-axis.
  //Each plane is transformed according to the view (axial, coronal and saggital) afterwards.
  localStorage->m_Plane->SetPoint1(planeBounds[1] , planeBounds[2], depth); //P1: (xMax, yMin, depth)
  localStorage->m_Plane->SetPoint2(planeBounds[0], planeBounds[3], depth); //P2: (xMin, yMax, depth)
}

float mitk::LabelSetImageVtkMapper2D::CalculateLayerDepth(mitk::BaseRenderer* renderer)
{
  //get the clipping range to check how deep into z direction we can render images
  double maxRange = renderer->GetVtkRenderer()->GetActiveCamera()->GetClippingRange()[1];

  //Due to a VTK bug, we cannot use the whole clipping range. /100 is empirically determined
  float depth = -maxRange*0.01; // divide by 100
  int layer = 0;
  GetDataNode()->GetIntProperty( "layer", layer, renderer);
  //add the layer property for each image to render images with a higher layer on top of the others
  depth += layer*10; //*10: keep some room for each image (e.g. for QBalls in between)
  if (depth > 0.0f)
  {
    depth = 0.0f;
    MITK_WARN << "Layer value exceeds clipping range. Set to minimum instead.";
  }
  return depth;
}

const mitk::LabelSetImage* mitk::LabelSetImageVtkMapper2D::GetInput( void )
{
  return static_cast< const mitk::LabelSetImage * >( GetDataNode()->GetData() );
}

vtkProp* mitk::LabelSetImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

void mitk::LabelSetImageVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::LabelSetImage *input = const_cast< mitk::LabelSetImage * >( this->GetInput() );

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
    localStorage->m_ReslicedImageMapper->SetInput( localStorage->m_EmptyPolyData );
    localStorage->m_ActiveLabelContourMapper->SetInput( NULL);
    return;
  }

  mitk::DataNode* datanode = this->GetDataNode();

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
  datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
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
  localStorage->m_ActiveLabelContour = this->CreateOutlinePolyData( renderer );

//  if (layer == activeLayer)
//  {
    bool contourAll = false;
    datanode->GetBoolProperty( "labelset.contour.all", contourAll, renderer );

    bool contourActive = false;
    datanode->GetBoolProperty( "labelset.contour.active", contourActive, renderer );
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
  datanode->GetFloatProperty( "labelset.contour.width", contourWidth, renderer );
  localStorage->m_ActiveLabelContourActor->GetProperty()->SetLineWidth( contourWidth );

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
  datanode->GetBoolProperty( "texture interpolation", textureInterpolation, renderer );

  //set the interpolation modus according to the property
  localStorage->m_Texture->SetInterpolate(textureInterpolation);

  localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowFilter->GetOutputPort());

  this->TransformActor( renderer );

  //we need the contour for the binary outline property as actor
  localStorage->m_ActiveLabelContourMapper->SetInput( localStorage->m_ActiveLabelContour );
//  localStorage->m_ActiveLabelContourActor->SetTexture(NULL); //no texture for contours

  //setup the textured plane
  this->GeneratePlane( renderer, sliceBounds );
  //set the plane as input for the mapper
  localStorage->m_ReslicedImageMapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
  //set the texture for the actor

  localStorage->m_ReslicedImageActor->SetTexture(localStorage->m_Texture);
}

void mitk::LabelSetImageVtkMapper2D::ApplyColor( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  mitk::LabelSetImage *input = dynamic_cast<mitk::LabelSetImage*>(this->GetDataNode()->GetData());
  int activeLayer = input->GetActiveLayer();
  const mitk::Color& activeLabelColor = input->GetActiveLabelColor( activeLayer );
  double rgbConv[3] = {(double)activeLabelColor.GetRed(), (double)activeLabelColor.GetGreen(), (double)activeLabelColor.GetBlue()};
  localStorage->m_ActiveLabelContourActor->GetProperty()->SetColor(rgbConv);
}

void mitk::LabelSetImageVtkMapper2D::ApplyOpacity( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity( opacity, renderer, "opacity" );
  localStorage->m_ReslicedImageActor->GetProperty()->SetOpacity(opacity);
  localStorage->m_ActiveLabelContourActor->GetProperty()->SetOpacity(opacity);
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

  mitk::LabelSetImage* image  = const_cast<mitk::LabelSetImage *>( this->GetInput() );
  if ( !image )
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
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
  mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (image.IsNull())
  {
    MITK_ERROR << "Setting default properties to a node without an image";
    return;
  }

  // Properties common for both images and segmentations
  node->AddProperty( "depthOffset", FloatProperty::New( 0.0 ), renderer, overwrite );
  if(image->IsRotated()) node->AddProperty( "reslice interpolation", VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC) );
  else node->AddProperty( "reslice interpolation", VtkResliceInterpolationProperty::New() );
  node->AddProperty( "texture interpolation", BoolProperty::New( mitk::DataNodeFactory::m_TextureInterpolationActive ) );  // set to user configurable default value (see global options)
  node->AddProperty( "in plane resample extent by geometry", BoolProperty::New( false ) );
  node->AddProperty( "bounding box", BoolProperty::New( false ) );

  mitk::RenderingModeProperty::Pointer renderingModeProperty = mitk::RenderingModeProperty::New( RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW );
  node->AddProperty( "Image Rendering.Mode", renderingModeProperty);

  mitk::ColormapProperty::Pointer colormapProperty = mitk::ColormapProperty::New();
  node->AddProperty( "colormap", colormapProperty, renderer, overwrite );
  colormapProperty->SetValue( ColormapProperty::CM_MULTILABEL );

  node->AddProperty( "opacity", FloatProperty::New(1.0f), renderer, overwrite );
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

mitk::LabelSetImageVtkMapper2D::LocalStorage* mitk::LabelSetImageVtkMapper2D::GetLocalStorage(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

vtkSmartPointer<vtkPolyData> mitk::LabelSetImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage(renderer);

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

  int activeLayer = this->GetInput()->GetActiveLayer();
  int activeLabel = this->GetInput()->GetActiveLabelIndex(activeLayer);

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
  localStorage->m_ReslicedImageActor->SetUserTransform(trans);
  //transform the origin to center based coordinates, because MITK is center based.
  localStorage->m_ReslicedImageActor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);
  localStorage->m_ActiveLabelContourActor->SetUserTransform(trans);
  localStorage->m_ActiveLabelContourActor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);
}

bool mitk::LabelSetImageVtkMapper2D::RenderingGeometryIntersectsImage( const Geometry2D* renderingGeometry, SlicedGeometry3D* imageGeometry )
{
  // if either one of the two geometries is NULL we return true
  // for safety reasons
  if ( renderingGeometry == NULL || imageGeometry == NULL )
    return true;

  // get the distance for the first cornerpoint
  ScalarType initialDistance = renderingGeometry->SignedDistance( imageGeometry->GetCornerPoint( 0 ) );
  for( int i=1; i<8; i++ )
  {
    mitk::Point3D cornerPoint = imageGeometry->GetCornerPoint( i );

    // get the distance to the other cornerpoints
    ScalarType distance = renderingGeometry->SignedDistance( cornerPoint );

    // if it has not the same signing as the distance of the first point
    if ( initialDistance * distance < 0 )
    {
      // we have an intersection and return true
      return true;
    }
  }

  // all distances have the same sign, no intersection and we return false
  return false;
}

mitk::LabelSetImageVtkMapper2D::LocalStorage::~LocalStorage()
{
}

mitk::LabelSetImageVtkMapper2D::LocalStorage::LocalStorage()
{

  m_LevelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();

  //Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  m_Texture = vtkSmartPointer<vtkNeverTranslucentTexture>::New().GetPointer();
  m_ReslicedImageMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_ReslicedImageActor = vtkSmartPointer<vtkActor>::New();
  m_ActiveLabelContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_ActiveLabelContourActor = vtkSmartPointer<vtkActor>::New();
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_Reslicer = mitk::ExtractSliceFilter::New();
//  m_LabelOutline = vtkSmartPointer<vtkImageLabelOutline>::New();
  m_ActiveLabelContour = vtkSmartPointer<vtkPolyData>::New();
  m_ReslicedImage = vtkSmartPointer<vtkImageData>::New();
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();
//  m_ImageBlend = vtkSmartPointer<vtkImageBlend>::New();

  //do not repeat the texture (the image)
  m_Texture->RepeatOff();

  //set corresponding mappers for the actors
  m_ReslicedImageActor->SetMapper( m_ReslicedImageMapper );
  m_ActiveLabelContourActor->SetMapper( m_ActiveLabelContourMapper );

  m_Actors->AddPart( m_ActiveLabelContourActor );
  m_Actors->AddPart( m_ReslicedImageActor );
}
