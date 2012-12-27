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
#include "mitkExtractSliceFilter.h"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkLinearTransform.h>
#include <vtkImageChangeInformation.h>
#include <mitkAbstractTransformGeometry.h>
#include <vtkGeneralTransform.h>


mitk::ExtractSliceFilter::ExtractSliceFilter(vtkImageReslice* reslicer ){

  if(reslicer == NULL){
    m_Reslicer = vtkSmartPointer<vtkImageReslice>::New();
  }
  else
  {
    m_Reslicer = reslicer;
  }

  m_TimeStep = 0;
  m_Reslicer->ReleaseDataFlagOn();
  m_InterpolationMode = ExtractSliceFilter::RESLICE_NEAREST;
  m_ResliceTransform = NULL;
  m_InPlaneResampleExtentByGeometry = false;
  m_OutPutSpacing = new mitk::ScalarType[2];
  m_OutputDimension = 2;
  m_ZSpacing = 1.0;
  m_ZMin = 0;
  m_ZMax = 0;
  m_VtkOutputRequested = false;

}

mitk::ExtractSliceFilter::~ExtractSliceFilter(){
  m_ResliceTransform = NULL;
  m_WorldGeometry = NULL;
  delete [] m_OutPutSpacing;
}

void mitk::ExtractSliceFilter::GenerateOutputInformation(){
  //TODO try figure out how to set the specs of the slice before it is actually extracted
  /*Image::Pointer output = this->GetOutput();
  Image::ConstPointer input = this->GetInput();
  if (input.IsNull()) return;
  unsigned int dimensions[2];
  dimensions[0] = m_WorldGeometry->GetExtent(0);
  dimensions[1] = m_WorldGeometry->GetExtent(1);
  output->Initialize(input->GetPixelType(), 2, dimensions, 1);*/
}

void mitk::ExtractSliceFilter::GenerateInputRequestedRegion(){
  //As we want all pixel information fo the image in our plane, the requested region
  //is set to the largest possible region in the image.
  //This is needed because an oblique plane has a larger extent then the image
  //and the in pipeline it is checked via PropagateResquestedRegion(). But the
  //extent of the slice is actually fitting because it is oblique within the image.
  ImageToImageFilter::InputImagePointer input =  const_cast< ImageToImageFilter::InputImageType* > ( this->GetInput() );
  input->SetRequestedRegionToLargestPossibleRegion();
}


mitk::ScalarType* mitk::ExtractSliceFilter::GetOutputSpacing(){
  return m_OutPutSpacing;
}


void mitk::ExtractSliceFilter::GenerateData(){

  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

  if (!input)
  {
    MITK_ERROR << "mitk::ExtractSliceFilter: No input image available. Please set the input!" << std::endl;
    itkExceptionMacro("mitk::ExtractSliceFilter: No input image available. Please set the input!");
    return;
  }

  if(!m_WorldGeometry)
  {
    MITK_ERROR << "mitk::ExtractSliceFilter: No Geometry for reslicing available." << std::endl;
    itkExceptionMacro("mitk::ExtractSliceFilter: No Geometry for reslicing available.");
    return;
  }


  const TimeSlicedGeometry *inputTimeGeometry = this->GetInput()->GetTimeSlicedGeometry();
  if ( ( inputTimeGeometry == NULL )
    || ( inputTimeGeometry->GetTimeSteps() == 0 ) )
  {
    itkWarningMacro(<<"Error reading input image TimeSlicedGeometry.");
    return;
  }

  // is it a valid timeStep?
  if ( inputTimeGeometry->IsValidTime( m_TimeStep ) == false )
  {
    itkWarningMacro(<<"This is not a valid timestep: "<< m_TimeStep );
    return;
  }

  // check if there is something to display.
  if ( ! input->IsVolumeSet( m_TimeStep ) )
  {
    itkWarningMacro(<<"No volume data existent at given timestep "<< m_TimeStep );
    return;
  }




  /*================#BEGIN setup vtkImageRslice properties================*/
  Point3D origin;
  Vector3D right, bottom, normal;
  double widthInMM, heightInMM;
  Vector2D extent;


  const PlaneGeometry* planeGeometry = dynamic_cast<const PlaneGeometry*>(m_WorldGeometry);


  if ( planeGeometry != NULL )
  {
    //if the worldGeomatry is a PlaneGeometry everthing is straight forward

    origin = planeGeometry->GetOrigin();
    right  = planeGeometry->GetAxisVector( 0 );
    bottom = planeGeometry->GetAxisVector( 1 );
    normal = planeGeometry->GetNormal();

    if ( m_InPlaneResampleExtentByGeometry )
    {
      // Resampling grid corresponds to the current world geometry. This
      // means that the spacing of the output 2D image depends on the
      // currently selected world geometry, and *not* on the image itself.
      extent[0] = m_WorldGeometry->GetExtent( 0 );
      extent[1] = m_WorldGeometry->GetExtent( 1 );
    }
    else
    {
      // Resampling grid corresponds to the input geometry. This means that
      // the spacing of the output 2D image is directly derived from the
      // associated input image, regardless of the currently selected world
      // geometry.
      Vector3D rightInIndex, bottomInIndex;
      inputTimeGeometry->GetGeometry3D( m_TimeStep )->WorldToIndex( right, rightInIndex );
      inputTimeGeometry->GetGeometry3D( m_TimeStep )->WorldToIndex( bottom, bottomInIndex );
      extent[0] = rightInIndex.GetNorm();
      extent[1] = bottomInIndex.GetNorm();
    }

    // Get the extent of the current world geometry and calculate resampling
    // spacing therefrom.
    widthInMM = m_WorldGeometry->GetExtentInMM( 0 );
    heightInMM = m_WorldGeometry->GetExtentInMM( 1 );


    m_OutPutSpacing[0] = widthInMM / extent[0];
    m_OutPutSpacing[1] = heightInMM / extent[1];


    right.Normalize();
    bottom.Normalize();
    normal.Normalize();


    /*
    * Transform the origin to center based coordinates.
    * Note:
    * This is needed besause vtk's origin is center based too (!!!) ( see 'The VTK book' page 88 )
    * and the worldGeometry surrouding the image is no imageGeometry. So the worldGeometry
    * has its origin at the corner of the voxel and needs to be transformed.
    */
    origin += right * ( m_OutPutSpacing[0] * 0.5 );
    origin += bottom * ( m_OutPutSpacing[1] * 0.5 );



    //set the tranform for reslicing.
    // Use inverse transform of the input geometry for reslicing the 3D image.
    // This is needed if the image volume already transformed
    if(m_ResliceTransform.IsNotNull())
      m_Reslicer->SetResliceTransform(m_ResliceTransform->GetVtkTransform()->GetLinearInverse());


    // Set background level to TRANSLUCENT (see Geometry2DDataVtkMapper3D),
    // else the background of the image turns out gray
    m_Reslicer->SetBackgroundLevel( -32768 );

  }
  else{
    //Code for curved planes, mostly taken 1:1 from imageVtkMapper2D and not tested yet.
    // Do we have an AbstractTransformGeometry?
    // This is the case for AbstractTransformGeometry's (e.g. a ThinPlateSplineCurvedGeometry )
    const mitk::AbstractTransformGeometry* abstractGeometry =
      dynamic_cast< const AbstractTransformGeometry * >(m_WorldGeometry);

    if(abstractGeometry != NULL)
    {
      m_ResliceTransform = abstractGeometry;

      extent[0] = abstractGeometry->GetParametricExtent(0);
      extent[1] = abstractGeometry->GetParametricExtent(1);

      widthInMM = abstractGeometry->GetParametricExtentInMM(0);
      heightInMM = abstractGeometry->GetParametricExtentInMM(1);

      m_OutPutSpacing[0] = widthInMM / extent[0];
      m_OutPutSpacing[1] = heightInMM / extent[1];

      origin = abstractGeometry->GetPlane()->GetOrigin();

      right = abstractGeometry->GetPlane()->GetAxisVector(0);
      right.Normalize();

      bottom = abstractGeometry->GetPlane()->GetAxisVector(1);
      bottom.Normalize();

      normal = abstractGeometry->GetPlane()->GetNormal();
      normal.Normalize();

      // Use a combination of the InputGeometry *and* the possible non-rigid
      // AbstractTransformGeometry for reslicing the 3D Image
      vtkSmartPointer<vtkGeneralTransform> composedResliceTransform = vtkGeneralTransform::New();
      composedResliceTransform->Identity();
      composedResliceTransform->Concatenate(
        inputTimeGeometry->GetGeometry3D( m_TimeStep )->GetVtkTransform()->GetLinearInverse() );
      composedResliceTransform->Concatenate(
        abstractGeometry->GetVtkAbstractTransform()
        );

      m_Reslicer->SetResliceTransform( composedResliceTransform );
      composedResliceTransform->UnRegister( NULL ); // decrease RC

      // Set background level to BLACK instead of translucent, to avoid
      // boundary artifacts (see Geometry2DDataVtkMapper3D)
      m_Reslicer->SetBackgroundLevel( -1023 );
    }
    else
    {
      itkExceptionMacro("mitk::ExtractSliceFilter: No fitting geometry for reslice axis!");
      return;
    }

  }

  if(m_ResliceTransform.IsNotNull()){
    //if the resliceTransform is set the reslice axis are recalculated.
    //Thus the geometry information is not fitting. Therefor a unitSpacingFilter
    //is used to set up a global spacing of 1 and compensate the transform.
    vtkSmartPointer<vtkImageChangeInformation> unitSpacingImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New() ;
    unitSpacingImageFilter->ReleaseDataFlagOn();

    unitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );
    unitSpacingImageFilter->SetInput( input->GetVtkImageData(m_TimeStep) );

    m_Reslicer->SetInput(unitSpacingImageFilter->GetOutput() );
  }
  else
  {
    //if no tranform is set the image can be used directly
    m_Reslicer->SetInput(input->GetVtkImageData(m_TimeStep));
  }


  /*setup the plane where vktImageReslice extracts the slice*/

  //ResliceAxesOrigin is the ancor point of the plane
  double originInVtk[3];
  itk2vtk(origin, originInVtk);
  m_Reslicer->SetResliceAxesOrigin(originInVtk);


  //the cosines define the plane: x and y are the direction vectors, n is the planes normal
  //this specifies a matrix 3x3
  //  x1 y1 n1
  //  x2 y2 n2
  //  x3 y3 n3
  double cosines[9];

  vnl2vtk(right.GetVnlVector(), cosines);//x

  vnl2vtk(bottom.GetVnlVector(), cosines + 3);//y

  vnl2vtk(normal.GetVnlVector(), cosines + 6);//n

  m_Reslicer->SetResliceAxesDirectionCosines(cosines);


  //we only have one slice, not a volume
  m_Reslicer->SetOutputDimensionality(m_OutputDimension);


  //set the interpolation mode for slicing
  switch(this->m_InterpolationMode){
    case RESLICE_NEAREST:
      m_Reslicer->SetInterpolationModeToNearestNeighbor();
      break;
    case RESLICE_LINEAR:
      m_Reslicer->SetInterpolationModeToLinear();
      break;
    case RESLICE_CUBIC:
      m_Reslicer->SetInterpolationModeToCubic();
    default:
      //the default interpolation used by mitk
      m_Reslicer->SetInterpolationModeToNearestNeighbor();
  }


  /*========== BEGIN setup extent of the slice ==========*/
  int xMin, xMax, yMin, yMax;

  xMin = yMin = 0;
  xMax = static_cast< int >( extent[0]);
  yMax = static_cast< int >( extent[1]);

  vtkFloatingPointType sliceBounds[6];
  if (m_WorldGeometry->GetReferenceGeometry())
  {
    for ( int i = 0; i < 6; ++i )
    {
      sliceBounds[i] = 0.0;
    }

    if (this->GetClippedPlaneBounds( m_WorldGeometry->GetReferenceGeometry(), planeGeometry, sliceBounds ))
    {
      // Calculate output extent (integer values)
      xMin = static_cast< int >( sliceBounds[0] / m_OutPutSpacing[0] + 0.5 );
      xMax = static_cast< int >( sliceBounds[1] / m_OutPutSpacing[0] + 0.5 );
      yMin = static_cast< int >( sliceBounds[2] / m_OutPutSpacing[1] + 0.5 );
      yMax = static_cast< int >( sliceBounds[3] / m_OutPutSpacing[1] + 0.5 );
    } // ELSE we use the default values
  }

  // Set the output extents! First included pixel index and last included pixel index
  // xMax and yMax are one after the last pixel. so they have to be decremented by 1.
  // In case we have a 2D image, xMax or yMax might be 0. in this case, do not decrement, but take 0.

  m_Reslicer->SetOutputExtent(xMin, std::max(0, xMax-1), yMin, std::max(0, yMax-1), m_ZMin, m_ZMax );
  /*========== END setup extent of the slice ==========*/


  m_Reslicer->SetOutputOrigin( 0.0, 0.0, 0.0 );

  m_Reslicer->SetOutputSpacing( m_OutPutSpacing[0], m_OutPutSpacing[1], m_ZSpacing );


  //TODO check the following lines, they are responsible wether vtk error outputs appear or not
  m_Reslicer->UpdateWholeExtent(); //this produces a bad allocation error for 2D images
  //m_Reslicer->GetOutput()->UpdateInformation();
  //m_Reslicer->GetOutput()->SetUpdateExtentToWholeExtent();

  //start the pipeline
  m_Reslicer->Update();

  /*================ #END setup vtkImageRslice properties================*/

  if(m_VtkOutputRequested){
    return;
    //no converting to mitk
    //no mitk geometry will be set, as the output is vtkImageData only!!!
  }
  else
  {
    /*================ #BEGIN Get the slice from vtkImageReslice and convert it to mit::Image================*/
    vtkImageData* reslicedImage;
    reslicedImage = m_Reslicer->GetOutput();


    if(!reslicedImage)
    {
      itkWarningMacro(<<"Reslicer returned empty image");
      return;
    }


    mitk::Image::Pointer resultImage = this->GetOutput();

    //initialize resultimage with the specs of the vtkImageData object returned from vtkImageReslice
    if (reslicedImage->GetDataDimension() == 1)
    {
       // If original image was 2D, the slice might have an y extent of 0.
       // Still i want to ensure here that Image is 2D
         resultImage->Initialize(reslicedImage,1,-1,-1,1);
    }
    else
    {
       resultImage->Initialize(reslicedImage);
    }


    //transfer the voxel data
    resultImage->SetVolume(reslicedImage->GetScalarPointer());


    //set the geometry from current worldgeometry for the reusultimage
    //this is needed that the image has the correct mitk geometry
    //the originalGeometry is the Geometry of the result slice
    AffineGeometryFrame3D::Pointer originalGeometryAGF = m_WorldGeometry->Clone();
    Geometry2D::Pointer originalGeometry = dynamic_cast<Geometry2D*>( originalGeometryAGF.GetPointer() );

    originalGeometry->GetIndexToWorldTransform()->SetMatrix(m_WorldGeometry->GetIndexToWorldTransform()->GetMatrix());

    //the origin of the worldGeometry is transformed to center based coordinates to be an imageGeometry
    Point3D sliceOrigin = originalGeometry->GetOrigin();

    sliceOrigin += right * ( m_OutPutSpacing[0] * 0.5 );
    sliceOrigin += bottom * ( m_OutPutSpacing[1] * 0.5 );

    //a worldGeometry is no imageGeometry, thus it is manually set to true
    originalGeometry->ImageGeometryOn();


    /*At this point we have to adjust the geometry because the origin isn't correct.
    The wrong origin is related to the rotation of the current world geometry plane.
    This causes errors on transfering world to index coordinates. We just shift the
    origin in each direction about the amount of the expanding (needed while rotating
    the plane).
    */
    Vector3D axis0 = originalGeometry->GetAxisVector(0);
    Vector3D axis1 = originalGeometry->GetAxisVector(1);
    axis0.Normalize();
    axis1.Normalize();


    //adapt the origin. Note that for orthogonal planes the minima are '0' and thus the origin stays the same.
    sliceOrigin += (axis0 * (xMin * m_OutPutSpacing[0])) + (axis1 * (yMin * m_OutPutSpacing[1]));

    originalGeometry->SetOrigin(sliceOrigin);

    originalGeometry->Modified();


    resultImage->SetGeometry( originalGeometry );


    /*the bounds as well as the extent of the worldGeometry are not adapted correctly during crosshair rotation.
    This is only a quick fix and has to be evaluated.
    The new bounds are set via the max values of the calcuted slice extent. It will look like [ 0, x, 0, y, 0, 1].
    */
    mitk::BoundingBox::BoundsArrayType boundsCopy;
    boundsCopy[0] = boundsCopy[2] = boundsCopy[4] = 0;
    boundsCopy[5] = 1;
    boundsCopy[1] =  xMax - xMin;
    boundsCopy[3] =  yMax - yMin;
    resultImage->GetGeometry()->SetBounds(boundsCopy);

    /*================ #END Get the slice from vtkImageReslice and convert it to mitk Image================*/
  }
}


bool mitk::ExtractSliceFilter::GetClippedPlaneBounds(vtkFloatingPointType bounds[6]){

  if(!m_WorldGeometry || !this->GetInput())
    return false;

  return this->GetClippedPlaneBounds(m_WorldGeometry->GetReferenceGeometry(), dynamic_cast< const PlaneGeometry * >( m_WorldGeometry ), bounds);

}


bool mitk::ExtractSliceFilter::GetClippedPlaneBounds( const Geometry3D *boundingGeometry,
                                                     const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds )
{
  bool b =  this->CalculateClippedPlaneBounds(boundingGeometry, planeGeometry, bounds);

  return b;
}


bool mitk::ExtractSliceFilter
::CalculateClippedPlaneBounds( const Geometry3D *boundingGeometry,
                              const PlaneGeometry *planeGeometry, vtkFloatingPointType *bounds )
{
  // Clip the plane with the bounding geometry. To do so, the corner points
  // of the bounding box are transformed by the inverse transformation
  // matrix, and the transformed bounding box edges derived therefrom are
  // clipped with the plane z=0. The resulting min/max values are taken as
  // bounds for the image reslicer.
  const BoundingBox *boundingBox = boundingGeometry->GetBoundingBox();

  BoundingBox::PointType bbMin = boundingBox->GetMinimum();
  BoundingBox::PointType bbMax = boundingBox->GetMaximum();

  vtkPoints *points = vtkPoints::New();
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

  vtkPoints *newPoints = vtkPoints::New();

  vtkTransform *transform = vtkTransform::New();
  transform->Identity();
  transform->Concatenate(
    planeGeometry->GetVtkTransform()->GetLinearInverse()
    );

  transform->Concatenate( boundingGeometry->GetVtkTransform() );

  transform->TransformPoints( points, newPoints );
  transform->Delete();

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

  // clean up vtk data
  points->Delete();
  newPoints->Delete();

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

bool mitk::ExtractSliceFilter
::LineIntersectZero( vtkPoints *points, int p1, int p2,
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
