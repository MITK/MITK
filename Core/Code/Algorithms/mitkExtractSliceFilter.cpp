/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkExtractSliceFilter.h"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkLinearTransform.h>
#include <vtkImageChangeInformation.h>


mitk::ExtractSliceFilter::ExtractSliceFilter(){
	m_Reslicer = vtkSmartPointer<vtkImageReslice>::New();
	m_TimeStep = 0;
	m_Reslicer->ReleaseDataFlagOn();//todo check what it does
	m_InterpolationMode = ExtractSliceFilter::RESLICE_NEAREST;
	m_ResliceTransform = NULL;
	m_InPlaneResampleExtentByGeometry = false;
}

mitk::ExtractSliceFilter::~ExtractSliceFilter(){
	
}

void mitk::ExtractSliceFilter::GenerateOutputInformation(){
	
	/*Image::Pointer output = this->GetOutput();
	Image::ConstPointer input = this->GetInput();
	if (input.IsNull()) return;
	unsigned int dimensions[2];
	dimensions[0] = m_WorldGeometry->GetExtent(0);
	dimensions[1] = m_WorldGeometry->GetExtent(1);
	output->Initialize(input->GetPixelType(), 2, dimensions, 1);*/
}

void mitk::ExtractSliceFilter::GenerateInputRequestedRegion(){
	ImageToImageFilter::InputImagePointer input =  const_cast< ImageToImageFilter::InputImageType* > ( this->GetInput() );
	input->SetRequestedRegionToLargestPossibleRegion();
}


void mitk::ExtractSliceFilter::GenerateData(){
	
	mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

	if ( !input){
		MITK_ERROR << "mitk::ExtractSliceFilter: No input image available. Please set the input!" << std::endl;
        itkExceptionMacro("mitk::ExtractSliceFilter: No input image available. Please set the input!");
        return;
    }

	if(!m_WorldGeometry){
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
	double mmPerPixel[2];
	

	const PlaneGeometry* planeGeometry = dynamic_cast<const PlaneGeometry*>(m_WorldGeometry);
	if ( planeGeometry != NULL )
	{
		origin = planeGeometry->GetOrigin();
		right  = planeGeometry->GetAxisVector( 0 ); // right = Extent of Image in mm (worldspace)
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
		

		mmPerPixel[0] = widthInMM / extent[0];
		mmPerPixel[1] = heightInMM / extent[1];


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
		origin += right * ( mmPerPixel[0] * 0.5 );
		origin += bottom * ( mmPerPixel[1] * 0.5 );
		 
		
		
		//set the tranform for reslicing.
		// Use inverse transform of the input geometry for reslicing the 3D image.
		// This is needed if the image volume already transformed 
		if(m_ResliceTransform)
			m_Reslicer->SetResliceTransform(m_ResliceTransform->GetVtkTransform()->GetLinearInverse());


		// Set background level to TRANSLUCENT (see Geometry2DDataVtkMapper3D),
		// else the background of the image turns out gray
		m_Reslicer->SetBackgroundLevel( -32768 );


		// Calculate the actual bounds of the transformed plane clipped by the
		// dataset bounding box; this is required for drawing the texture at the
		// correct position during 3D mapping.
		//boundsInitialized = this->CalculateClippedPlaneBounds(
		//	m_WorldGeometry->GetReferenceGeometry(), planeGeometry, sliceBounds );

	}  
  ////Code for curved planes
  // else{
  //  // Do we have an AbstractTransformGeometry?
  //  // This is the case for AbstractTransformGeometry's (e.g. a thin-plate-spline transform)
  //  const mitk::AbstractTransformGeometry* abstractGeometry =
  //      dynamic_cast< const AbstractTransformGeometry * >(worldGeometry);

  //  if(abstractGeometry != NULL)
  //  {

  //    extent[0] = abstractGeometry->GetParametricExtent(0);
  //    extent[1] = abstractGeometry->GetParametricExtent(1);

  //    widthInMM = abstractGeometry->GetParametricExtentInMM(0);
  //    heightInMM = abstractGeometry->GetParametricExtentInMM(1);

  //    localStorage->m_mmPerPixel[0] = widthInMM / extent[0];
  //    localStorage->m_mmPerPixel[1] = heightInMM / extent[1];

  //    localStorage->m_Origin = abstractGeometry->GetPlane()->GetOrigin();

  //    localStorage->m_Right = abstractGeometry->GetPlane()->GetAxisVector(0);
  //    localStorage->m_Right.Normalize();

  //    localStorage->m_Bottom = abstractGeometry->GetPlane()->GetAxisVector(1);
  //    localStorage->m_Bottom.Normalize();

  //    localStorage->m_Normal = abstractGeometry->GetPlane()->GetNormal();
  //    localStorage->m_Normal.Normalize();

  //    // Use a combination of the InputGeometry *and* the possible non-rigid
  //    // AbstractTransformGeometry for reslicing the 3D Image
  //    vtkGeneralTransform *composedResliceTransform = vtkGeneralTransform::New();
  //    composedResliceTransform->Identity();
  //    composedResliceTransform->Concatenate(
  //        inputGeometry->GetVtkTransform()->GetLinearInverse() );
  //    composedResliceTransform->Concatenate(
  //        abstractGeometry->GetVtkAbstractTransform()
  //        );

  //    localStorage->m_Reslicer->SetResliceTransform( composedResliceTransform );
  //    composedResliceTransform->UnRegister( NULL ); // decrease RC

  //    // Set background level to BLACK instead of translucent, to avoid
  //    // boundary artifacts (see Geometry2DDataVtkMapper3D)
  //    localStorage->m_Reslicer->SetBackgroundLevel( -1023 );
  //  }
  //}

	vtkImageChangeInformation * unitSpacingImageFilter = vtkImageChangeInformation::New() ;
	unitSpacingImageFilter->ReleaseDataFlagOn();
	
	unitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );
	unitSpacingImageFilter->SetInput( input->GetVtkImageData(m_TimeStep) );

	m_Reslicer->SetInput(unitSpacingImageFilter->GetOutput() );
	/*m_Reslicer->SetInput(input->GetVtkImageData(m_TimeStep));*/


	/*setup the plane where vktImageReslice extracts the slice*/

	//ResliceAxesOrigin is the ancor point of the plane
	double originArray[3];
	itk2vtk(origin, originArray);
	m_Reslicer->SetResliceAxesOrigin(originArray);


	//the cosines define the plane: x and y are the direction vectors, n is the planes normal
	//this specifies a matrix 3x3
	//	x1 y1 n1
	//	x2 y2 n2
	//	x3 y3 n3
	double cosines[9];

	vnl2vtk(right.GetVnlVector(), cosines);//x

	vnl2vtk(bottom.GetVnlVector(), cosines + 3);//y

	Vector3D normalVector = CrossProduct(m_WorldGeometry->GetAxisVector(0), m_WorldGeometry->GetAxisVector(1));
	normalVector.Normalize();
	vnl2vtk(normal.GetVnlVector(), cosines + 6);//n

	m_Reslicer->SetResliceAxesDirectionCosines(cosines);	


	//we only have one slice, not a volume
	m_Reslicer->SetOutputDimensionality(2);


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
	

	int xMin, xMax, yMin, yMax;
	bool boundsInitialized = false;
	if(m_WorldGeometry->GetReferenceGeometry()){
		vtkFloatingPointType sliceBounds[6];
		
		for ( int i = 0; i < 6; ++i )
		{
			sliceBounds[i] = 0.0;
		}
		this->CalculateClippedPlaneBounds( m_WorldGeometry->GetReferenceGeometry(), planeGeometry, sliceBounds );
		
		// Calculate output extent (integer values)
		xMin = static_cast< int >( sliceBounds[0] / mmPerPixel[0] + 0.5 );
		xMax = static_cast< int >( sliceBounds[1] / mmPerPixel[0] + 0.5 );
		yMin = static_cast< int >( sliceBounds[2] / mmPerPixel[1] + 0.5 );
		yMax = static_cast< int >( sliceBounds[3] / mmPerPixel[1] + 0.5 );
		
	}else
	{
		// If no reference geometry is available, we also don't know about the
		// maximum plane size;
		xMin = yMin = 0;
		xMax = static_cast< int >( extent[0]);
		yMax = static_cast< int >( extent[1]);
	}

	m_Reslicer->SetOutputExtent(xMin, xMax-1, yMin, yMax-1, 0, 0 );


	
	m_Reslicer->SetOutputOrigin( 0.0, 0.0, 0.0 );
	m_Reslicer->SetOutputSpacing( mmPerPixel[0], mmPerPixel[1], 1.0 );
	//m_Reslicer->SetOutputSpacing(m_WorldGeometry->GetSpacing()[0],m_WorldGeometry->GetSpacing()[1], 1.0);
	//m_Reslicer->SetOutputSpacing( 1.0, 1.0, 1.0 );



	//start the pipeline
	m_Reslicer->Modified();
	m_Reslicer->Update();

/*================ #END setup vtkImageRslice properties================*/



/*================ #BEGIN Get the slice from vtkImageReslice and convert it to mitk Image================*/
	vtkImageData* reslicedImage;
	reslicedImage = m_Reslicer->GetOutput();



	if(!reslicedImage)
	{
		itkWarningMacro(<<"Reslicer returned empty image");
		return;
	}


	mitk::Image::Pointer resultImage = this->GetOutput();

	//initialize resultimage with the specs of the vtkImageData object returned from vtkImageReslice
	resultImage->Initialize(reslicedImage);

	//transfer the voxel data
	resultImage->SetVolume(reslicedImage->GetScalarPointer());	

	//set the geometry from current worldgeometry for the reusultimage
	//this is needed that the image has the correct mitk geometry
	AffineGeometryFrame3D::Pointer originalGeometryAGF = m_WorldGeometry->Clone();
	Geometry2D::Pointer originalGeometry = dynamic_cast<Geometry2D*>( originalGeometryAGF.GetPointer() );
	originalGeometry->ChangeImageGeometryConsideringOriginOffset(true);
    resultImage->SetGeometry( originalGeometry );

	resultImage->GetGeometry()->TransferItkToVtkTransform();

	
	resultImage->GetGeometry()->Modified();
/*================ #END Get the slice from vtkImageReslice and convert it to mitk Image================*/
}


bool mitk::ExtractSliceFilter::GetBounds(vtkFloatingPointType bounds[6]){
	
	return this->CalculateClippedPlaneBounds(m_WorldGeometry->GetReferenceGeometry(), dynamic_cast< const PlaneGeometry * >( m_WorldGeometry ), bounds); 
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
	BoundingBox::PointType bbCenter = boundingBox->GetCenter();

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