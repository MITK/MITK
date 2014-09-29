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

#include "mitkExtractDirectedPlaneImageFilter.h"
#include "mitkAbstractTransformGeometry.h"
//#include "mitkImageMapperGL2D.h"

#include <mitkProperties.h>
#include <mitkDataNode.h>
#include <mitkResliceMethodProperty.h>
#include "vtkMitkThickSlicesFilter.h"

#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkImageChangeInformation.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>


mitk::ExtractDirectedPlaneImageFilter::ExtractDirectedPlaneImageFilter()
: m_WorldGeometry(NULL)
{
  MITK_WARN << "Class ExtractDirectedPlaneImageFilter is deprecated! Use ExtractSliceFilter instead.";

  m_Reslicer = vtkImageReslice::New();

  m_TargetTimestep = 0;
  m_InPlaneResampleExtentByGeometry = true;
  m_ResliceInterpolationProperty = NULL;//VtkResliceInterpolationProperty::New(); //TODO initial with value
    m_ThickSlicesMode = 0;
  m_ThickSlicesNum = 1;
}

mitk::ExtractDirectedPlaneImageFilter::~ExtractDirectedPlaneImageFilter()
{
  if(m_ResliceInterpolationProperty!=NULL)m_ResliceInterpolationProperty->Delete();
  m_Reslicer->Delete();
}

void mitk::ExtractDirectedPlaneImageFilter::GenerateData()
{
  // A world geometry must be set...
  if ( m_WorldGeometry == NULL )
  {
    itkWarningMacro(<<"No world geometry has been set. Returning.");
    return;
  }

  Image *input = const_cast< ImageToImageFilter::InputImageType* >( this->GetInput() );
  input->Update();

  if ( input == NULL )
  {
    itkWarningMacro(<<"No input set.");
    return;
  }

  const TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  if ( ( inputTimeGeometry == NULL )
    || ( inputTimeGeometry->CountTimeSteps() == 0 ) )
  {
    itkWarningMacro(<<"Error reading input image geometry.");
    return;
  }

  // Get the target timestep; if none is set, use the lowest given.
  unsigned int timestep = m_TargetTimestep;

  if ( inputTimeGeometry->IsValidTimeStep( timestep ) == false )
  {
    itkWarningMacro(<<"This is not a valid timestep: "<<timestep);
    return;
  }

  // check if there is something to display.
  if ( ! input->IsVolumeSet( timestep ) )
  {
    itkWarningMacro(<<"No volume data existent at given timestep "<<timestep);
    return;
  }

  Image::RegionType requestedRegion = input->GetLargestPossibleRegion();
  requestedRegion.SetIndex( 3, timestep );
  requestedRegion.SetSize( 3, 1 );
  requestedRegion.SetSize( 4, 1 );
  input->SetRequestedRegion( &requestedRegion );
  input->Update();

  vtkImageData* inputData = input->GetVtkImageData( timestep );

  if ( inputData == NULL )
  {
    itkWarningMacro(<<"Could not extract vtk image data for given timestep"<<timestep);
    return;
  }

  double spacing[3];
  inputData->GetSpacing( spacing );

  // how big the area is in physical coordinates: widthInMM x heightInMM pixels
  mitk::ScalarType widthInMM, heightInMM;

  // where we want to sample
  Point3D origin;
  Vector3D right, bottom, normal;
  Vector3D rightInIndex, bottomInIndex;

  assert( input->GetTimeGeometry() == inputTimeGeometry );

  // take transform of input image into account
  BaseGeometry* inputGeometry = inputTimeGeometry->GetGeometryForTimeStep( timestep );
  if ( inputGeometry == NULL )
  {
    itkWarningMacro(<<"There is no Geometry3D at given timestep "<<timestep);
    return;
  }

  ScalarType mmPerPixel[2];

  // Bounds information for reslicing (only required if reference geometry
  // is present)
  double bounds[6];
  bool boundsInitialized = false;

  for ( int i = 0; i < 6; ++i )
  {
    bounds[i] = 0.0;
  }

  Vector2D extent; extent.Fill( 0.0 );

  // Do we have a simple PlaneGeometry?
  if ( dynamic_cast< const PlaneGeometry * >( m_WorldGeometry ) != NULL &&
       dynamic_cast< const AbstractTransformGeometry * >( m_WorldGeometry ) == NULL)
  {
    const PlaneGeometry *planeGeometry =
      static_cast< const PlaneGeometry * >( m_WorldGeometry );
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
      inputGeometry->WorldToIndex( right, rightInIndex );
      inputGeometry->WorldToIndex( bottom, bottomInIndex );
      extent[0] = rightInIndex.GetNorm();
      extent[1] = bottomInIndex.GetNorm();
    }

    // Get the extent of the current world geometry and calculate resampling
    // spacing therefrom.
    widthInMM = m_WorldGeometry->GetExtentInMM( 0 );
    heightInMM = m_WorldGeometry->GetExtentInMM( 1 );

    mmPerPixel[0] = widthInMM / extent[0];
    mmPerPixel[1] = heightInMM / extent[1];

    right.Normalize();
    bottom.Normalize();
    normal.Normalize();

    //origin += right * ( mmPerPixel[0] * 0.5 );
    //origin += bottom * ( mmPerPixel[1] * 0.5 );

    //widthInMM -= mmPerPixel[0];
    //heightInMM -= mmPerPixel[1];

    // Use inverse transform of the input geometry for reslicing the 3D image
    m_Reslicer->SetResliceTransform(
      inputGeometry->GetVtkTransform()->GetLinearInverse() );

    // Set background level to TRANSLUCENT (see PlaneGeometryDataVtkMapper3D)
    m_Reslicer->SetBackgroundLevel( -32768 );

    // Check if a reference geometry does exist (as would usually be the case for
    // PlaneGeometry).
    // Note: this is currently not strictly required, but could facilitate
    // correct plane clipping.
    if ( m_WorldGeometry->GetReferenceGeometry() )
    {
      // Calculate the actual bounds of the transformed plane clipped by the
      // dataset bounding box; this is required for drawing the texture at the
      // correct position during 3D mapping.
      boundsInitialized = this->CalculateClippedPlaneBounds(
        m_WorldGeometry->GetReferenceGeometry(), planeGeometry, bounds );
    }
  }

  // Do we have an AbstractTransformGeometry?
  else if ( dynamic_cast< const AbstractTransformGeometry * >( m_WorldGeometry ) )
  {
    const mitk::AbstractTransformGeometry* abstractGeometry =
      dynamic_cast< const AbstractTransformGeometry * >(m_WorldGeometry);

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

    m_Reslicer->SetResliceTransform( composedResliceTransform );

    // Set background level to BLACK instead of translucent, to avoid
    // boundary artifacts (see PlaneGeometryDataVtkMapper3D)
    m_Reslicer->SetBackgroundLevel( -1023 );
    composedResliceTransform->Delete();
  }
  else
  {
    itkWarningMacro(<<"World Geometry has to be a PlaneGeometry or an AbstractTransformGeometry.");
    return;
  }

  // Make sure that the image to be resliced has a certain minimum size.
  if ( (extent[0] <= 2) && (extent[1] <= 2) )
  {
    itkWarningMacro(<<"Image is too small to be resliced...");
    return;
  }

  vtkSmartPointer<vtkImageChangeInformation> unitSpacingImageFilter = vtkImageChangeInformation::New() ;
  unitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );
  unitSpacingImageFilter->SetInputData( inputData );

  m_Reslicer->SetInputConnection( unitSpacingImageFilter->GetOutputPort() );

  //m_Reslicer->SetInput( inputData );

  m_Reslicer->SetOutputDimensionality( 2 );
  m_Reslicer->SetOutputOrigin( 0.0, 0.0, 0.0 );

  Vector2D pixelsPerMM;
  pixelsPerMM[0] = 1.0 / mmPerPixel[0];
  pixelsPerMM[1] = 1.0 / mmPerPixel[1];

  //calulate the originArray and the orientations for the reslice-filter
  double originArray[3];
  itk2vtk( origin, originArray );

  m_Reslicer->SetResliceAxesOrigin( originArray );

  double cosines[9];

  // direction of the X-axis of the sampled result
  vnl2vtk( right.GetVnlVector(), cosines );

  // direction of the Y-axis of the sampled result
  vnl2vtk( bottom.GetVnlVector(), cosines + 3 );

  // normal of the plane
  vnl2vtk( normal.GetVnlVector(), cosines + 6 );

  m_Reslicer->SetResliceAxesDirectionCosines( cosines );

  int xMin, xMax, yMin, yMax;
  if ( boundsInitialized )
  {
    xMin = static_cast< int >( bounds[0] / mmPerPixel[0] );//+ 0.5 );
    xMax = static_cast< int >( bounds[1] / mmPerPixel[0] );//+ 0.5 );
    yMin = static_cast< int >( bounds[2] / mmPerPixel[1] );//+ 0.5);
    yMax = static_cast< int >( bounds[3] / mmPerPixel[1] );//+ 0.5 );
  }
  else
  {
    // If no reference geometry is available, we also don't know about the
    // maximum plane size; so the overlap is just ignored

    xMin = yMin = 0;
    xMax = static_cast< int >( extent[0] - pixelsPerMM[0] );//+ 0.5 );
    yMax = static_cast< int >( extent[1] - pixelsPerMM[1] );//+ 0.5 );
  }

  m_Reslicer->SetOutputSpacing( mmPerPixel[0], mmPerPixel[1], 1.0 );
  // xMax and yMax are meant exclusive until now, whereas
  // SetOutputExtent wants an inclusive bound. Thus, we need
  // to subtract 1.
  m_Reslicer->SetOutputExtent( xMin, xMax-1, yMin, yMax-1, 0, 1 );

  // Do the reslicing. Modified() is called to make sure that the reslicer is
  // executed even though the input geometry information did not change; this
  // is necessary when the input /em data, but not the /em geometry changes.
  m_Reslicer->Modified();
  m_Reslicer->ReleaseDataFlagOn();

  m_Reslicer->Update();

  // 1. Check the result
  vtkImageData* reslicedImage = m_Reslicer->GetOutput();

  if((reslicedImage == NULL) || (reslicedImage->GetDataDimension() < 1))
  {
    itkWarningMacro(<<"Reslicer returned empty image");
    return;
  }

  unsigned int dimensions[2];
  dimensions[0] = (unsigned int)extent[0]; dimensions[1] = (unsigned int)extent[1];
  Vector3D spacingVector;
  FillVector3D(spacingVector, mmPerPixel[0], mmPerPixel[1], 1.0);

  mitk::Image::Pointer resultImage = this->GetOutput();
  resultImage->Initialize(input->GetPixelType(), 2, dimensions );
  resultImage->SetSpacing( spacingVector );
}


void mitk::ExtractDirectedPlaneImageFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}


bool mitk::ExtractDirectedPlaneImageFilter
::CalculateClippedPlaneBounds( const BaseGeometry *boundingGeometry,
                const PlaneGeometry *planeGeometry, double *bounds )
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
    const mitk::Vector3D planeSpacing = planeGeometry->GetSpacing();
    bounds[0] *= planeSpacing[0];
    bounds[1] *= planeSpacing[0];
    bounds[2] *= planeSpacing[1];
    bounds[3] *= planeSpacing[1];
    bounds[4] *= planeSpacing[2];
    bounds[5] *= planeSpacing[2];
    return true;
  }
}

bool mitk::ExtractDirectedPlaneImageFilter
::LineIntersectZero( vtkPoints *points, int p1, int p2,
          double *bounds )
{
  double point1[3];
  double point2[3];
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
