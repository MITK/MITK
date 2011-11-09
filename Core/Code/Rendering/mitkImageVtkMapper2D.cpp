/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//MITK
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkDataNodeFactory.h>
#include <mitkImageSliceSelector.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkResliceMethodProperty.h>
#include <mitkTimeSlicedGeometry.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkPixelType.h>
//#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

//MITK Rendering
#include "mitkImageVtkMapper2D.h"
#include "vtkMitkThickSlicesFilter.h"
#include "vtkMitkApplyLevelWindowToRGBFilter.h"

//VTK
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkGeneralTransform.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>

//ITK
#include <itkRGBAPixel.h>

mitk::ImageVtkMapper2D::ImageVtkMapper2D()
{
}

mitk::ImageVtkMapper2D::~ImageVtkMapper2D()
{
  //The 3D RW Mapper (Geometry2DDataVtkMapper3D) is listening to this event,
  //in order to delete the images from the 3D RW.
  this->InvokeEvent( itk::DeleteEvent() );
}

//set the two points defining the textured plane according to the dimension and spacing
void mitk::ImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6])
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  //Set the origin to (xMin; yMin; depth) of the plane. This is necessary for obtaining the correct
  //plane size in crosshair rotation and swivel mode.

  float depth = this->CalculateLayerDepth(renderer);

  localStorage->m_Plane->SetOrigin(planeBounds[0], planeBounds[2], depth);
  //These two points define the axes of the plane in combination with the origin.
  //Point 1 is the x-axis and point 2 the y-axis.
  //Each plane is transformed according to the view (transversal, coronal and saggital) afterwards.
  localStorage->m_Plane->SetPoint1(planeBounds[1], planeBounds[2], depth); //P1: (xMax, yMin, depth)
  localStorage->m_Plane->SetPoint2(planeBounds[0], planeBounds[3], depth); //P2: (xMin, yMax, depth)
}

float mitk::ImageVtkMapper2D::CalculateLayerDepth(mitk::BaseRenderer* renderer)
{
  //get the clipping range to check how deep into z direction we can render images
  double maxRange = renderer->GetVtkRenderer()->GetActiveCamera()->GetClippingRange()[1];

  //Due to a VTK bug, we cannot use the whole clipping range. /100 is empirically determined
  float depth = -maxRange*0.01; // divide by 100
  int layer = 0;
  GetDataNode()->GetIntProperty( "layer", layer, renderer);
  //add the layer property for each image to render images with a higher layer on top of the others
  depth += layer*10; //*10: keep some room for each image (e.g. for QBalls in between)
  if(depth > 0.0f) {
    depth = 0.0f;
    MITK_WARN << "Layer value exceeds clipping range. Set to minimum instead.";
  }
  return depth;
}

const mitk::Image* mitk::ImageVtkMapper2D::GetInput( void )
{
  return static_cast< const mitk::Image * >( this->GetData() );
}

vtkProp* mitk::ImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{  
  //  this->Update(renderer);
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actor;
}

void mitk::ImageVtkMapper2D::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible( renderer )==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

void mitk::ImageVtkMapper2D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
    return;
  if ( GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

  if ( input == NULL )
  {
    return;
  }

  //check if there is a valid worldGeometry
  const Geometry2D *worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( ( worldGeometry == NULL ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
  {
    return;
  }

  // check if there is something to display
  if ( !input->IsVolumeSet( this->GetTimestep() ) ) return;

  input->Update();

  vtkImageData* inputData = input->GetVtkImageData( this->GetTimestep() );
  if ( inputData == NULL )
  {
    return;
  }

  // how big the area is in physical coordinates: widthInMM x heightInMM pixels
  mitk::ScalarType widthInMM, heightInMM;

  // where we want to sample
  Point3D origin;
  Vector3D right, bottom, normal;

  // take transform of input image into account
  const TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  const Geometry3D* inputGeometry = inputTimeGeometry->GetGeometry3D( this->GetTimestep() );

  //World spacing
  ScalarType mmPerPixel[2];

  // Bounds information for reslicing (only reuqired if reference geometry 
  // is present)
  vtkFloatingPointType sliceBounds[6];
  bool boundsInitialized = false;
  for ( int i = 0; i < 6; ++i )
  {
    sliceBounds[i] = 0.0;
  }

  //Extent (in pixels) of the image
  Vector2D extent;

  // Do we have a simple PlaneGeometry?
  // This is the "regular" case (e.g. slicing through an image axis-parallel or even oblique)
  const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( worldGeometry );
  if ( planeGeometry != NULL )
  {
    origin = planeGeometry->GetOrigin();
    right  = planeGeometry->GetAxisVector( 0 ); // right = Extent of Image in mm (worldspace)
    bottom = planeGeometry->GetAxisVector( 1 );
    normal = planeGeometry->GetNormal();

    bool inPlaneResampleExtentByGeometry = false;
    GetDataNode()->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);

    if ( inPlaneResampleExtentByGeometry )
    {
      // Resampling grid corresponds to the current world geometry. This
      // means that the spacing of the output 2D image depends on the
      // currently selected world geometry, and *not* on the image itself.
      extent[0] = worldGeometry->GetExtent( 0 );
      extent[1] = worldGeometry->GetExtent( 1 );
    }
    else
    {
      // Resampling grid corresponds to the input geometry. This means that
      // the spacing of the output 2D image is directly derived from the
      // associated input image, regardless of the currently selected world
      // geometry.
      Vector3D rightInIndex, bottomInIndex;
      inputGeometry->WorldToIndex( right, rightInIndex );
      inputGeometry->WorldToIndex( bottom, bottomInIndex );
      extent[0] = rightInIndex.GetNorm();
      extent[1] = bottomInIndex.GetNorm();
    }
    
    // Get the extent of the current world geometry and calculate resampling
    // spacing therefrom.
    widthInMM = worldGeometry->GetExtentInMM( 0 );
    heightInMM = worldGeometry->GetExtentInMM( 1 );

    mmPerPixel[0] = widthInMM / extent[0];
    mmPerPixel[1] = heightInMM / extent[1];

    right.Normalize();
    bottom.Normalize();
    normal.Normalize();

    //transform the origin to corner based coordinates, because VTK is corner based.
    origin += right * ( mmPerPixel[0] * 0.5 );
    origin += bottom * ( mmPerPixel[1] * 0.5 );

    // Use inverse transform of the input geometry for reslicing the 3D image
    localStorage->m_Reslicer->SetResliceTransform(
        inputGeometry->GetVtkTransform()->GetLinearInverse() );

    // Set background level to TRANSLUCENT (see Geometry2DDataVtkMapper3D)
    localStorage->m_Reslicer->SetBackgroundLevel( -32768 );

    // Calculate the actual bounds of the transformed plane clipped by the
    // dataset bounding box; this is required for drawing the texture at the
    // correct position during 3D mapping.
    boundsInitialized = this->CalculateClippedPlaneBounds(
        worldGeometry->GetReferenceGeometry(), planeGeometry, sliceBounds );
  }
  else
  {
    // Do we have an AbstractTransformGeometry?
    // This is the case for AbstractTransformGeometry's (e.g. a thin-plate-spline transform)
    const mitk::AbstractTransformGeometry* abstractGeometry =
        dynamic_cast< const AbstractTransformGeometry * >(worldGeometry);

    if(abstractGeometry != NULL)
    {

      extent[0] = abstractGeometry->GetParametricExtent(0);
      extent[1] = abstractGeometry->GetParametricExtent(1);

      widthInMM = abstractGeometry->GetParametricExtentInMM(0);
      heightInMM = abstractGeometry->GetParametricExtentInMM(1);

      mmPerPixel[0] = widthInMM / extent[0];
      mmPerPixel[1] = heightInMM / extent[1];

      origin = abstractGeometry->GetPlane()->GetOrigin();

      right = abstractGeometry->GetPlane()->GetAxisVector(0);
      right.Normalize();

      bottom = abstractGeometry->GetPlane()->GetAxisVector(1);
      bottom.Normalize();

      normal = abstractGeometry->GetPlane()->GetNormal();
      normal.Normalize();

      // Use a combination of the InputGeometry *and* the possible non-rigid
      // AbstractTransformGeometry for reslicing the 3D Image
      vtkGeneralTransform *composedResliceTransform = vtkGeneralTransform::New();
      composedResliceTransform->Identity();
      composedResliceTransform->Concatenate(
          inputGeometry->GetVtkTransform()->GetLinearInverse() );
      composedResliceTransform->Concatenate(
          abstractGeometry->GetVtkAbstractTransform()
          );

      localStorage->m_Reslicer->SetResliceTransform( composedResliceTransform );
      composedResliceTransform->UnRegister( NULL ); // decrease RC

      // Set background level to BLACK instead of translucent, to avoid
      // boundary artifacts (see Geometry2DDataVtkMapper3D)
      localStorage->m_Reslicer->SetBackgroundLevel( -1023 );
    }
    else
    {
      //no geometry => we can't reslice
      return;
    }
  }

  // Make sure that the image to display has a certain minimum size.
  if ( (extent[0] <= 2) && (extent[1] <= 2) )
  {
    return;
  }

  //### begin set reslice interpolation
  // Initialize the interpolation mode for resampling; switch to nearest
  // neighbor if the input image is too small.
  if ( (input->GetDimension() >= 3) && (input->GetDimension(2) > 1) )
  {
    VtkResliceInterpolationProperty *resliceInterpolationProperty;
    this->GetDataNode()->GetProperty(
        resliceInterpolationProperty, "reslice interpolation" );

    int interpolationMode = VTK_RESLICE_NEAREST;
    if ( resliceInterpolationProperty != NULL )
    {
      interpolationMode = resliceInterpolationProperty->GetInterpolation();
    }

    switch ( interpolationMode )
    {
    case VTK_RESLICE_NEAREST:
      localStorage->m_Reslicer->SetInterpolationModeToNearestNeighbor();
      break;
    case VTK_RESLICE_LINEAR:
      localStorage->m_Reslicer->SetInterpolationModeToLinear();
      break;
    case VTK_RESLICE_CUBIC:
      localStorage->m_Reslicer->SetInterpolationModeToCubic();
      break;
    }
  }
  else
  {
    localStorage->m_Reslicer->SetInterpolationModeToNearestNeighbor();
  }
  //### end set reslice interpolation

  //Thickslicing
  int thickSlicesMode = 0;
  int thickSlicesNum = 1;

  // Thick slices parameters
  if( inputData->GetNumberOfScalarComponents() == 1 ) // for now only single component are allowed
  {
    DataNode *dn=renderer->GetCurrentWorldGeometry2DNode();
    if(dn)
    {
      ResliceMethodProperty *resliceMethodEnumProperty=0;

      if( dn->GetProperty( resliceMethodEnumProperty, "reslice.thickslices" ) && resliceMethodEnumProperty )
        thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

      IntProperty *intProperty=0;
      if( dn->GetProperty( intProperty, "reslice.thickslices.num" ) && intProperty )
      {
        thickSlicesNum = intProperty->GetValue();
        if(thickSlicesNum < 1) thickSlicesNum=1;
        if(thickSlicesNum > 10) thickSlicesNum=10;
      }
    }
    else
    {
      MITK_WARN << "no associated widget plane data tree node found";
    }
  }

  localStorage->m_UnitSpacingImageFilter->SetInput( inputData );
  localStorage->m_Reslicer->SetInput( localStorage->m_UnitSpacingImageFilter->GetOutput() );

  //number of pixels per mm in x- and y-direction of the resampled
  Vector2D pixelsPerMM;
  pixelsPerMM[0] = 1.0 / mmPerPixel[0];
  pixelsPerMM[1] = 1.0 / mmPerPixel[1];

  //calulate the originArray and the orientations for the reslice-filter
  double originArray[3];
  itk2vtk( origin, originArray );

  localStorage->m_Reslicer->SetResliceAxesOrigin( originArray );

  double cosines[9];

  // direction of the X-axis of the sampled result
  vnl2vtk( right.Get_vnl_vector(), cosines );

  // direction of the Y-axis of the sampled result
  vnl2vtk( bottom.Get_vnl_vector(), cosines + 3 );//fill next 3 elements

  // normal of the plane
  vnl2vtk( normal.Get_vnl_vector(), cosines + 6 );//fill the last 3 elements

  localStorage->m_Reslicer->SetResliceAxesDirectionCosines( cosines );

  int xMin, xMax, yMin, yMax;
  if ( boundsInitialized )
  {
    // Calculate output extent (integer values)
    xMin = static_cast< int >( sliceBounds[0] / mmPerPixel[0] + 0.5 );
    xMax = static_cast< int >( sliceBounds[1] / mmPerPixel[0] + 0.5 );
    yMin = static_cast< int >( sliceBounds[2] / mmPerPixel[1] + 0.5 );
    yMax = static_cast< int >( sliceBounds[3] / mmPerPixel[1] + 0.5 );
  }
  else
  {
    // If no reference geometry is available, we also don't know about the
    // maximum plane size;
    xMin = yMin = 0;
    xMax = static_cast< int >( extent[0]
                               - pixelsPerMM[0] + 0.5);
    yMax = static_cast< int >( extent[1]
                               - pixelsPerMM[1] + 0.5);
  }

  // Disallow huge dimensions
  if ( (xMax-xMin) * (yMax-yMin) > 4096*4096 )
  {
    return;
  }

  // Calculate dataset spacing in plane z direction (NOT spacing of current
  // world geometry)
  double dataZSpacing = 1.0;

  Vector3D normInIndex;
  inputGeometry->WorldToIndex( normal, normInIndex );

  if(thickSlicesMode > 0)
  {
    dataZSpacing = 1.0 / normInIndex.GetNorm();
    localStorage->m_Reslicer->SetOutputDimensionality( 3 );
    localStorage->m_Reslicer->SetOutputExtent( xMin, xMax-1, yMin, yMax-1, -thickSlicesNum, 0+thickSlicesNum );
  }
  else
  {
    localStorage->m_Reslicer->SetOutputDimensionality( 2 );
    localStorage->m_Reslicer->SetOutputExtent( xMin, xMax-1, yMin, yMax-1, 0, 0 );
  }

  localStorage->m_Reslicer->SetOutputOrigin( 0.0, 0.0, 0.0 );
  localStorage->m_Reslicer->SetOutputSpacing( mmPerPixel[0], mmPerPixel[1], dataZSpacing );
  // xMax and yMax are meant exclusive until now, whereas
  // SetOutputExtent wants an inclusive bound. Thus, we need
  // to subtract 1.

  vtkImageData* reslicedImage = 0;
  // Do the reslicing. Modified() is called to make sure that the reslicer is
  // executed even though the input geometry information did not change; this
  // is necessary when the input /em data, but not the /em geometry changes.
  if(thickSlicesMode>0)
  {
    localStorage->m_TSFilter->SetThickSliceMode( thickSlicesMode-1 );
    localStorage->m_TSFilter->SetInput( localStorage->m_Reslicer->GetOutput() );
    localStorage->m_TSFilter->Modified();
    localStorage->m_TSFilter->Update();
    reslicedImage = localStorage->m_TSFilter->GetOutput();
  }
  else
  {
    localStorage->m_Reslicer->Modified();
    localStorage->m_Reslicer->Update();
    reslicedImage = localStorage->m_Reslicer->GetOutput();
  }

  if((reslicedImage == NULL) || (reslicedImage->GetDataDimension() < 1))
  {
    MITK_WARN << "reslicer returned empty image";
    return;
  }

  //set the current slice for the localStorage
  //  localStorage->m_ReslicedImage = reslicedImage;
  localStorage->m_ReslicedImage->DeepCopy( reslicedImage );

  //set the current slice as texture for the plane
  localStorage->m_Texture->SetInput(localStorage->m_ReslicedImage);

  //setup the textured plane
  this->GeneratePlane( renderer, sliceBounds );

  //apply the properties after the slice was set
  this->ApplyProperties( renderer, mmPerPixel );

  //get the transformation matrix of the reslicer in order to render the slice as transversal, coronal or saggital
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_Reslicer->GetResliceAxes();

  //transform the origin to center based coordinates, because MITK is center based.
  Point3D originCenterBased = origin;
  originCenterBased -= right * ( mmPerPixel[0] * 0.5 );
  originCenterBased -= bottom * ( mmPerPixel[1] * 0.5 );

  matrix->SetElement(0, 3, originCenterBased[0]);
  matrix->SetElement(1, 3, originCenterBased[1]);
  matrix->SetElement(2, 3, originCenterBased[2]);
  trans->SetMatrix(matrix);

  //transform the plane/contour (the actual actor) to the corresponding view (transversal, coronal or saggital)
  localStorage->m_Actor->SetUserTransform(trans);

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}

bool mitk::ImageVtkMapper2D::LineIntersectZero( vtkPoints *points, int p1, int p2,
                                                vtkFloatingPointType *bounds )
{
  vtkFloatingPointType point1[3];
  vtkFloatingPointType point2[3];
  points->GetPoint( p1, point1 );
  points->GetPoint( p2, point2 );

  if ( (point1[2] * point2[2] <= 0.0) && (point1[2] != point2[2]) )
  {
    double x, y;
    x = ( point1[0] * point2[2] - point1[2] * point2[0] ) / ( point2[2] - point1[2] );
    y = ( point1[1] * point2[2] - point1[2] * point2[1] ) / ( point2[2] - point1[2] );

    if ( x < bounds[0] ) { bounds[0] = x; }
    if ( x > bounds[1] ) { bounds[1] = x; }
    if ( y < bounds[2] ) { bounds[2] = y; }
    if ( y > bounds[3] ) { bounds[3] = y; }
    bounds[4] = bounds[5] = 0.0;
    return true;
  }
  return false;
}

bool mitk::ImageVtkMapper2D::CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry,
                                                          const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds )
{
  // Clip the plane with the bounding geometry. To do so, the corner points
  // of the bounding box are transformed by the inverse transformation
  // matrix, and the transformed bounding box edges derived therefrom are
  // clipped with the plane z=0. The resulting min/max values are taken as
  // bounds for the image reslicer.
  const mitk::BoundingBox *boundingBox = boundingGeometry->GetBoundingBox();

  mitk::BoundingBox::PointType bbMin = boundingBox->GetMinimum();
  mitk::BoundingBox::PointType bbMax = boundingBox->GetMaximum();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  if(boundingGeometry->GetImageGeometry())
  {
    points->InsertPoint( 0, bbMin[0]-0.5, bbMin[1]-0.5, bbMin[2]-0.5 );
    points->InsertPoint( 1, bbMin[0]-0.5, bbMin[1]-0.5, bbMax[2]-0.5 );
    points->InsertPoint( 2, bbMin[0]-0.5, bbMax[1]-0.5, bbMax[2]-0.5 );
    points->InsertPoint( 3, bbMin[0]-0.5, bbMax[1]-0.5, bbMin[2]-0.5 );
    points->InsertPoint( 4, bbMax[0]-0.5, bbMin[1]-0.5, bbMin[2]-0.5 );
    points->InsertPoint( 5, bbMax[0]-0.5, bbMin[1]-0.5, bbMax[2]-0.5 );
    points->InsertPoint( 6, bbMax[0]-0.5, bbMax[1]-0.5, bbMax[2]-0.5 );
    points->InsertPoint( 7, bbMax[0]-0.5, bbMax[1]-0.5, bbMin[2]-0.5 );
  }
  else
  {
    points->InsertPoint( 0, bbMin[0], bbMin[1], bbMin[2] );
    points->InsertPoint( 1, bbMin[0], bbMin[1], bbMax[2] );
    points->InsertPoint( 2, bbMin[0], bbMax[1], bbMax[2] );
    points->InsertPoint( 3, bbMin[0], bbMax[1], bbMin[2] );
    points->InsertPoint( 4, bbMax[0], bbMin[1], bbMin[2] );
    points->InsertPoint( 5, bbMax[0], bbMin[1], bbMax[2] );
    points->InsertPoint( 6, bbMax[0], bbMax[1], bbMax[2] );
    points->InsertPoint( 7, bbMax[0], bbMax[1], bbMin[2] );
  }

  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Identity();
  transform->Concatenate( planeGeometry->GetVtkTransform()->GetLinearInverse() );

  transform->Concatenate( boundingGeometry->GetVtkTransform() );

  transform->TransformPoints( points, newPoints );

  bounds[0] = bounds[2] = 10000000.0;
  bounds[1] = bounds[3] = -10000000.0;
  bounds[4] = bounds[5] = 0.0;

  this->LineIntersectZero( newPoints, 0, 1, bounds );
  this->LineIntersectZero( newPoints, 1, 2, bounds );
  this->LineIntersectZero( newPoints, 2, 3, bounds );
  this->LineIntersectZero( newPoints, 3, 0, bounds );
  this->LineIntersectZero( newPoints, 0, 4, bounds );
  this->LineIntersectZero( newPoints, 1, 5, bounds );
  this->LineIntersectZero( newPoints, 2, 6, bounds );
  this->LineIntersectZero( newPoints, 3, 7, bounds );
  this->LineIntersectZero( newPoints, 4, 5, bounds );
  this->LineIntersectZero( newPoints, 5, 6, bounds );
  this->LineIntersectZero( newPoints, 6, 7, bounds );
  this->LineIntersectZero( newPoints, 7, 4, bounds );

  if ( (bounds[0] > 9999999.0) || (bounds[2] > 9999999.0)
    || (bounds[1] < -9999999.0) || (bounds[3] < -9999999.0) )
    {
    return false;
  }
  else
  {
    // The resulting bounds must be adjusted by the plane spacing, since we
    // we have so far dealt with index coordinates
    const float *planeSpacing = planeGeometry->GetFloatSpacing();
    bounds[0] *= planeSpacing[0];
    bounds[1] *= planeSpacing[0];
    bounds[2] *= planeSpacing[1];
    bounds[3] *= planeSpacing[1];
    bounds[4] *= planeSpacing[2];
    bounds[5] *= planeSpacing[2];
    return true;
  }
}

void mitk::ImageVtkMapper2D::ApplyProperties(mitk::BaseRenderer* renderer, mitk::ScalarType mmPerPixel[2])
{
  //get the current localStorage for the corresponding renderer
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check for interpolation properties
  bool textureInterpolation = false;
  GetDataNode()->GetBoolProperty( "texture interpolation", textureInterpolation, renderer );

  //set the interpolation modus according to the property
  localStorage->m_Texture->SetInterpolate(textureInterpolation);

  //do not repeat the texture (the image)
  localStorage->m_Texture->RepeatOff();

  float rgb[3]= { 1.0f, 1.0f, 1.0f };
  float opacity = 1.0f;

  // check for opacity prop and use it for rendering if it exists
  GetOpacity( opacity, renderer );
  //set the opacity according to the properties
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);

  // check for color prop and use it for rendering if it exists
  // binary image hovering & binary image selection
  bool hover    = false;
  bool selected = false;
  GetDataNode()->GetBoolProperty("binaryimage.ishovering", hover, renderer);
  GetDataNode()->GetBoolProperty("selected", selected, renderer);
  if(hover && !selected)
  {
    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
                                                                                ("binaryimage.hoveringcolor", renderer));
    if(colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
    }
    else
    {
      GetColor( rgb, renderer );
    }

  }
  if(selected)
  {
    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
                                                                                ("binaryimage.selectedcolor", renderer));
    if(colorprop.IsNotNull()) {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
    }
    else
    {
      GetColor( rgb, renderer );
    }

  }
  if(!hover && !selected)
  {
    GetColor( rgb, renderer );
  }

  //get the binary property
  bool binary = false;
  this->GetDataNode()->GetBoolProperty( "binary", binary, renderer );
  localStorage->m_Texture->SetMapColorScalarsThroughLookupTable(binary);

  //use color means that we want to use the color from the property list and not a lookuptable
  bool useColor = true;
  this->GetDataNode()->GetBoolProperty( "use color", useColor, renderer );

  //the finalLookuptable will be used for the rendering and can either be a user-defined table or the default lut
  vtkSmartPointer<vtkLookupTable> finalLookuptable = localStorage->m_LookupTable;

  //BEGIN PROPERTY user-defined lut
  //currently we do not allow a lookuptable if it is a binary image
  bool useDefaultLut = true;
  if((!useColor) && (!binary))
  {
    // If lookup table use is requested...
    mitk::LookupTableProperty::Pointer LookupTableProp;
    LookupTableProp = dynamic_cast<mitk::LookupTableProperty*>
                      (this->GetDataNode()->GetProperty("LookupTable"));
    //...check if there is a lookuptable provided by the user
    if ( LookupTableProp.IsNotNull() )
    {
      // If lookup table use is requested and supplied by the user:
      // only update the lut, when the properties have changed...
      if( LookupTableProp->GetLookupTable()->GetMTime()
        <= this->GetDataNode()->GetPropertyList()->GetMTime() )
        {
        LookupTableProp->GetLookupTable()->ChangeOpacityForAll( opacity );
        LookupTableProp->GetLookupTable()->ChangeOpacity(0, 0.0);
      }
      //we use the user-defined lookuptable
      finalLookuptable = LookupTableProp->GetLookupTable()->GetVtkLookupTable();
      //we obtained a user-defined lut and dont have to use the default table
      useDefaultLut = false;
    }
  }//END PROPERTY user-defined lut

  //use the finalLookuptable for mapping the colors
  localStorage->m_Texture->SetLookupTable( finalLookuptable );

  if((useDefaultLut) && (!useColor))
  { //the color function can be applied if the user does not want to use color
    //and does not provide a lookuptable
    ApplyColorTransferFunction(renderer);
  }

  //check if we need the default table
  if( useDefaultLut )
  {
    double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; //conversion to double for VTK
    localStorage->m_Actor->GetProperty()->SetColor(rgbConv);
  }
  else
  {
    //If the user defines a lut, we dont want to use the color and take white instead.
    localStorage->m_Actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
  }

  bool binaryOutline = false;
  this->GetDataNode()->GetBoolProperty( "outline binary", binaryOutline, renderer );
  if ( binary )
  {
    finalLookuptable->SetRange(0.0, 1.0);
    //0 is already mapped to transparent.
    //1 is now mapped to the current color and alpha

    if ( this->GetInput()->GetPixelType().GetBpe() <= 8 )
    {
      if (binaryOutline)
      {
        //generate ontours/outlines TODO: not always necessary
        localStorage->m_OutlinePolyData = CreateOutlinePolyData(renderer ,localStorage->m_ReslicedImage, mmPerPixel);

        float binaryOutlineWidth(1.0);
        if (this->GetDataNode()->GetFloatProperty( "outline width", binaryOutlineWidth, renderer ))
        {
          localStorage->m_Actor->GetProperty()->SetLineWidth(binaryOutlineWidth);
        }
      }
    }
    else
    {
      MITK_WARN << "Type of all binary images should be (un)signed char. Outline does not work on other pixel types!";
    }
  } //END binary image handling
  else
  {
    LevelWindow levelWindow;
    this->GetLevelWindow( levelWindow, renderer );

    //set up the lookuptable with the level window range
    finalLookuptable->SetRange( levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound() );

    mitk::PixelType pixelType = this->GetInput()->GetPixelType();
    if( pixelType.GetBitsPerComponent() == pixelType.GetBpe() ) //gray images with just one component
    {
      localStorage->m_Texture->MapColorScalarsThroughLookupTableOn();
    }
    else //RGB, RBGA or other images tpyes with more components
    {
      // obtain and apply opacity level window if possible
      localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();
      localStorage->m_LevelWindowToRGBFilterObject->SetLookupTable(localStorage->m_Texture->GetLookupTable());
      mitk::LevelWindow opacLevelWindow;
      if( this->GetLevelWindow( opacLevelWindow, renderer, "opaclevelwindow" ) )
      {
        localStorage->m_LevelWindowToRGBFilterObject->SetMinOpacity(opacLevelWindow.GetLowerWindowBound());
        localStorage->m_LevelWindowToRGBFilterObject->SetMaxOpacity(opacLevelWindow.GetUpperWindowBound());
      }
      else
      {
        localStorage->m_LevelWindowToRGBFilterObject->SetMinOpacity(0.0);
        localStorage->m_LevelWindowToRGBFilterObject->SetMaxOpacity(255.0);
      }
      localStorage->m_LevelWindowToRGBFilterObject->SetInput(localStorage->m_ReslicedImage);
      localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowToRGBFilterObject->GetOutputPort());
    }
  }

  if(binaryOutline && binary)
  {
    //We need the contour for the binary oultine property as actor
    localStorage->m_Mapper->SetInput(localStorage->m_OutlinePolyData);
    localStorage->m_Actor->SetTexture(NULL); //no texture for contours
  }
  else
  {
    //set the plane as input for the mapper
    localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
    //set the texture for the actor
    localStorage->m_Actor->SetTexture(localStorage->m_Texture);
  }
}

void mitk::ImageVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  if ( !this->IsVisible( renderer ) )
  {
    return;
  }

  mitk::Image* data  = const_cast<mitk::Image *>( this->GetInput() );
  if ( data == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const TimeSlicedGeometry *dataTimeGeometry = data->GetTimeSlicedGeometry();
  if ( ( dataTimeGeometry == NULL )
    || ( dataTimeGeometry->GetTimeSteps() == 0 )
    || ( !dataTimeGeometry->IsValidTime( this->GetTimestep() ) ) )
    {
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
    || (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) //Was the data modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) //was the geometry modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime())
    || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
    || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
    {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::ImageVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

  // Properties common for both images and segmentations
  node->AddProperty( "use color", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "depthOffset", mitk::FloatProperty::New( 0.0 ), renderer, overwrite );
  node->AddProperty( "outline binary", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "outline width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );
  if(image->IsRotated()) node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC) );
  else node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->AddProperty( "texture interpolation", mitk::BoolProperty::New( mitk::DataNodeFactory::m_TextureInterpolationActive ) );  // set to user configurable default value (see global options)
  node->AddProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->AddProperty( "bounding box", mitk::BoolProperty::New( false ) );

  std::string modality;
  if ( node->GetStringProperty( "dicom.series.Modality", modality ) )
  {
    // modality provided by DICOM or other reader
    if ( modality == "PT") // NOT a typo, PT is the abbreviation for PET used in DICOM
    {
      node->SetProperty( "use color", mitk::BoolProperty::New( false ), renderer );
      node->SetProperty( "opacity", mitk::FloatProperty::New( 0.5 ), renderer );
    }
  }

  bool isBinaryImage(false);
  if ( ! node->GetBoolProperty("binary", isBinaryImage) )
  {

    // ok, property is not set, use heuristic to determine if this
    // is a binary image
    mitk::Image::Pointer centralSliceImage;
    ScalarType minValue = 0.0;
    ScalarType maxValue = 0.0;
    ScalarType min2ndValue = 0.0;
    ScalarType max2ndValue = 0.0;
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();

    sliceSelector->SetInput(image);
    sliceSelector->SetSliceNr(image->GetDimension(2)/2);
    sliceSelector->SetTimeNr(image->GetDimension(3)/2);
    sliceSelector->SetChannelNr(image->GetDimension(4)/2);
    sliceSelector->Update();
    centralSliceImage = sliceSelector->GetOutput();
    if ( centralSliceImage.IsNotNull() && centralSliceImage->IsInitialized() )
    {
      minValue    = centralSliceImage->GetScalarValueMin();
      maxValue    = centralSliceImage->GetScalarValueMax();
      min2ndValue = centralSliceImage->GetScalarValue2ndMin();
      max2ndValue = centralSliceImage->GetScalarValue2ndMax();
    }
    if ( minValue == maxValue )
    {
      // centralSlice is strange, lets look at all data
      minValue    = image->GetScalarValueMin();
      maxValue    = image->GetScalarValueMaxNoRecompute();
      min2ndValue = image->GetScalarValue2ndMinNoRecompute();
      max2ndValue = image->GetScalarValue2ndMaxNoRecompute();
    }
    isBinaryImage = ( maxValue == min2ndValue && minValue == max2ndValue );
  }

  // some more properties specific for a binary...
  if (isBinaryImage)
  {
    node->AddProperty( "opacity", mitk::FloatProperty::New(0.3f), renderer, overwrite );
    node->AddProperty( "color", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.selectedcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.selectedannotationcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.hoveringcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.hoveringannotationcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binary", mitk::BoolProperty::New( true ), renderer, overwrite );
    node->AddProperty("layer", mitk::IntProperty::New(10), renderer, overwrite);
  }
  else          //...or image type object
  {
    node->AddProperty( "opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite );
    node->AddProperty( "color", ColorProperty::New(1.0,1.0,1.0), renderer, overwrite );
    node->AddProperty( "binary", mitk::BoolProperty::New( false ), renderer, overwrite );
    node->AddProperty("layer", mitk::IntProperty::New(0), renderer, overwrite);
  }

  if(image.IsNotNull() && image->IsInitialized())
  {
    if((overwrite) || (node->GetProperty("levelwindow", renderer)==NULL))
    {
      /* initialize level/window from DICOM tags */
      std::string sLevel;
      std::string sWindow;
      if (   node->GetStringProperty( "dicom.voilut.WindowCenter", sLevel )
          && node->GetStringProperty( "dicom.voilut.WindowWidth", sWindow ) )
      {
        float level = atof( sLevel.c_str() );
        float window = atof( sWindow.c_str() );

        mitk::LevelWindow contrast;
        std::string sSmallestPixelValueInSeries;
        std::string sLargestPixelValueInSeries;

        if (    node->GetStringProperty( "dicom.series.SmallestPixelValueInSeries", sSmallestPixelValueInSeries )
            && node->GetStringProperty( "dicom.series.LargestPixelValueInSeries", sLargestPixelValueInSeries ) )
        {
          float smallestPixelValueInSeries = atof( sSmallestPixelValueInSeries.c_str() );
          float largestPixelValueInSeries = atof( sLargestPixelValueInSeries.c_str() );
          contrast.SetRangeMinMax( smallestPixelValueInSeries-1, largestPixelValueInSeries+1 ); // why not a little buffer? 
                                                                                               // might remedy some l/w widget challenges
        }
        else
        {
          contrast.SetAuto( static_cast<mitk::Image*>(node->GetData()), false, true ); // we need this as a fallback
        }

        contrast.SetLevelWindow( level, window);
        node->SetProperty( "levelwindow", LevelWindowProperty::New( contrast ), renderer );
      }
    }
    if(((overwrite) || (node->GetProperty("opaclevelwindow", renderer)==NULL))
      && (image->GetPixelType().GetItkTypeId() && *(image->GetPixelType().GetItkTypeId()) == typeid(itk::RGBAPixel<unsigned char>)))
      {
      mitk::LevelWindow opaclevwin;
      opaclevwin.SetRangeMinMax(0,255);
      opaclevwin.SetWindowBounds(0,255);
      mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(opaclevwin);
      node->SetProperty( "opaclevelwindow", prop, renderer );
    }
    if((overwrite) || (node->GetProperty("LookupTable", renderer)==NULL))
    {
      // add a default rainbow lookup table for color mapping
      mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
      vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
      vtkLut->SetHueRange(0.6667, 0.0);
      vtkLut->SetTableRange(0.0, 20.0);
      vtkLut->Build();
      mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
      mitkLutProp->SetLookupTable(mitkLut);
      node->SetProperty( "LookupTable", mitkLutProp );
    }
  }
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkSmartPointer<vtkPolyData> mitk::ImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer* renderer, vtkSmartPointer<vtkImageData> binarySlice, mitk::ScalarType mmPerPixel[2]){
  int* dims = binarySlice->GetDimensions(); //dimensions of the image
  int line = dims[0]; //how many pixels per line?
  int x = 0; //pixel index x
  int y = 0; //pixel index y
  char* currentPixel;
  int nn = dims[0]*dims[1]; //max pixel(n,n)

  //get the depth for each contour
  float depth = CalculateLayerDepth(renderer);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); //the lines to connect the points
  for (int ii = 0; ii<nn; ii++) { //current pixel(i,i)
    currentPixel = static_cast<char*>(binarySlice->GetScalarPointer(x, y, 0));
    //if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel != 0)) {
      //check in which direction a line is necessary
      if (ii >= line && *(currentPixel-line) == 0) { //x direction - bottom edge of the pixel
        //add the 2 points
        vtkIdType p1 = points->InsertNextPoint(x*mmPerPixel[0], y*mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*mmPerPixel[0], y*mmPerPixel[1], depth);
        //add the line between both points
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
      if (ii <= nn-line && *(currentPixel+line) == 0) { //x direction - top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*mmPerPixel[0], (y+1)*mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*mmPerPixel[0], (y+1)*mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
      if (ii > 1 && *(currentPixel-1) == 0) { //y direction - left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*mmPerPixel[0], y*mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*mmPerPixel[0], (y+1)*mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
      if (ii < nn-1 && *(currentPixel+1) == 0) { //y direction - right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*mmPerPixel[0], y*mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*mmPerPixel[0], (y+1)*mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
    }

    //reached end of line
    x++;
    if (x >= line) {
      x = 0;
      y++;
    }
  }
  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  return polyData;
}

void mitk::ImageVtkMapper2D::ApplyColorTransferFunction(mitk::BaseRenderer* renderer)
{
  mitk::TransferFunctionProperty::Pointer transferFunctionProperty =
      dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataNode()->GetProperty("Image Rendering.Transfer Function",renderer ));
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  if(transferFunctionProperty.IsNotNull())
  {
    localStorage->m_Texture->SetLookupTable(transferFunctionProperty->GetValue()->GetColorTransferFunction());
  }
  else
  {
    MITK_WARN << "Neither a lookuptable nor a transfer function is set and use color is off.";
  }
}

mitk::ImageVtkMapper2D::LocalStorage::LocalStorage()
{
  //Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  m_Texture = vtkSmartPointer<vtkTexture>::New();
  m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Reslicer = vtkSmartPointer<vtkImageReslice>::New();
  m_TSFilter = vtkSmartPointer<vtkMitkThickSlicesFilter>::New();
  m_UnitSpacingImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_ReslicedImage = vtkSmartPointer<vtkImageData>::New();

  //the following actions are always the same and thus can be performed
  //in the constructor for each image (i.e. the image-corresponding local storage)
  m_TSFilter->ReleaseDataFlagOn();
  m_Reslicer->ReleaseDataFlagOn();

  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );

  //built a default lookuptable
  m_LookupTable->SetRampToLinear();
  m_LookupTable->SetSaturationRange( 0.0, 0.0 );
  m_LookupTable->SetHueRange( 0.0, 0.0 );
  m_LookupTable->SetValueRange( 0.0, 1.0 );
  m_LookupTable->Build();
  //map all black values to transparent
  m_LookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);

  //set the mapper for the actor
  m_Actor->SetMapper(m_Mapper);

  //filter for RGB(A) images
  m_LevelWindowToRGBFilterObject = new vtkMitkApplyLevelWindowToRGBFilter();
}
