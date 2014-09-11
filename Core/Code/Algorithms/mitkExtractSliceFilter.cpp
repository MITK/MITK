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
#include <mitkPlaneClipping.h>

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
  Superclass::GenerateOutputInformation();
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

  const TimeGeometry* inputTimeGeometry = this->GetInput()->GetTimeGeometry();
  if ( ( inputTimeGeometry == NULL )
    || ( inputTimeGeometry->CountTimeSteps() <= 0 ) )
  {
    itkWarningMacro(<<"Error reading input image TimeGeometry.");
    return;
  }

  // is it a valid timeStep?
  if ( inputTimeGeometry->IsValidTimeStep( m_TimeStep ) == false )
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
    vtkSmartPointer<vtkGeneralTransform> composedResliceTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    composedResliceTransform->Identity();
    composedResliceTransform->Concatenate(
      inputTimeGeometry->GetGeometryForTimeStep( m_TimeStep )->GetVtkTransform()->GetLinearInverse() );
    composedResliceTransform->Concatenate(
      abstractGeometry->GetVtkAbstractTransform()
      );

    m_Reslicer->SetResliceTransform( composedResliceTransform );

    // Set background level to BLACK instead of translucent, to avoid
    // boundary artifacts (see PlaneGeometryDataVtkMapper3D)
    m_Reslicer->SetBackgroundLevel( -1023 );
  }
  else{
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
        inputTimeGeometry->GetGeometryForTimeStep( m_TimeStep )->WorldToIndex( right, rightInIndex );
        inputTimeGeometry->GetGeometryForTimeStep( m_TimeStep )->WorldToIndex( bottom, bottomInIndex );
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

      // Set background level to TRANSLUCENT (see PlaneGeometryDataVtkMapper3D),
      // else the background of the image turns out gray
      m_Reslicer->SetBackgroundLevel( -32768 );
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
    unitSpacingImageFilter->SetInputData( input->GetVtkImageData(m_TimeStep) );

    m_Reslicer->SetInputConnection(unitSpacingImageFilter->GetOutputPort() );
  }
  else
  {
    //if no tranform is set the image can be used directly
    m_Reslicer->SetInputData(input->GetVtkImageData(m_TimeStep));
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
    break;
  default:
    //the default interpolation used by mitk
    m_Reslicer->SetInterpolationModeToNearestNeighbor();
  }

  /*========== BEGIN setup extent of the slice ==========*/
  int xMin, xMax, yMin, yMax;

  xMin = yMin = 0;
  xMax = static_cast< int >( extent[0]);
  yMax = static_cast< int >( extent[1]);

  double sliceBounds[6];
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

    //    mitk::AffineGeometryFrame3D::Pointer originalGeometryAGF = m_WorldGeometry->Clone();
    //    PlaneGeometry::Pointer originalGeometry = dynamic_cast<PlaneGeometry*>( originalGeometryAGF.GetPointer() );
    PlaneGeometry::Pointer originalGeometry = m_WorldGeometry->Clone();

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

bool mitk::ExtractSliceFilter::GetClippedPlaneBounds(double bounds[6]){
  if(!m_WorldGeometry || !this->GetInput())
    return false;

  return this->GetClippedPlaneBounds(m_WorldGeometry->GetReferenceGeometry(), dynamic_cast< const PlaneGeometry * >( m_WorldGeometry ), bounds);
}

bool mitk::ExtractSliceFilter::GetClippedPlaneBounds( const BaseGeometry *boundingGeometry,
                                                     const PlaneGeometry *planeGeometry, double *bounds )
{
  bool b =  mitk::PlaneClipping::CalculateClippedPlaneBounds(boundingGeometry, planeGeometry, bounds);

  return b;
}
