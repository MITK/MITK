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


#include "mitkExtractDirectedPlaneImageFilter.h"
#include "mitkAbstractTransformGeometry.h"
//#include "mitkImageMapperGL2D.h"

#include <mitkProperties.h>
#include <mitkDataNode.h>
#include <mitkDataNodeFactory.h>
#include <mitkResliceMethodProperty.h>
#include "vtkMitkThickSlicesFilter.h"

#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkImageChangeInformation.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

#include "pic2vtk.h"



mitk::ExtractDirectedPlaneImageFilter::ExtractDirectedPlaneImageFilter()
: m_WorldGeometry(NULL)
{
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
	mitk::Point3D origin;
	mitk::Vector3D right, bottom, normal;
	mitk::ScalarType mmPerPixel[2];
	
	
	mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

	if ( !input )
    {
        MITK_ERROR << "mitk::ExtractDirectedPlaneImageFilterNew: No input available. Please set the input!" << std::endl;
        itkExceptionMacro("mitk::ExtractDirectedPlaneImageFilterNew: No input available. Please set the input!");
        return;
    }

	const TimeSlicedGeometry *inputTimeGeometry = this->GetInput()->GetTimeSlicedGeometry();
	if ( ( inputTimeGeometry == NULL )
		|| ( inputTimeGeometry->GetTimeSteps() == 0 ) )
	{
		itkWarningMacro(<<"Error reading input image geometry.");
		return;
	}

	// A world geometry must be set...
	if ( m_WorldGeometry == NULL )
	{
		itkWarningMacro(<<"No world geometry has been set. Returning.");
		return;
	}

	//Get the target timestep; if none is set, use the lowest given.
	unsigned int timestep = 0;
	if ( ! m_TargetTimestep )
	{
		ScalarType time = m_WorldGeometry->GetTimeBounds()[0];
		if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
		{
		  timestep = inputTimeGeometry->MSToTimeStep( time );
		}
	 }
	 else timestep = m_TargetTimestep;

	 if ( inputTimeGeometry->IsValidTime( timestep ) == false )
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

	vtkImageData* inputData = input->GetVtkImageData( timestep );

	if ( inputData == NULL )
	{
		return;
	}


	// how big the area is in physical coordinates: widthInMM x heightInMM pixels
	mitk::ScalarType widthInMM, heightInMM;

	// take transform of input image into account
	const Geometry3D* inputGeometry = inputTimeGeometry->GetGeometry3D( timestep );

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
	const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( m_WorldGeometry );
	if ( planeGeometry != NULL )
	{
		origin = planeGeometry->GetOrigin();
		right  = planeGeometry->GetAxisVector( 0 ); // right = Extent of Image in mm (worldspace)
		bottom = planeGeometry->GetAxisVector( 1 );
		normal = planeGeometry->GetNormal();

		if ( m_InPlaneResampleExtentByGeometry )// TODO what is default value??
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
		
		//transform the origin to corner based coordinates, because VTK is corner based.
		origin += right * ( mmPerPixel[0] * 0.5 );
		origin += bottom * ( mmPerPixel[1] * 0.5 );
		
		

		// Use inverse transform of the input geometry for reslicing the 3D image
		//m_Reslicer->SetResliceTransform(
		//	m_WorldGeometry->GetVtkTransform()->GetLinearInverse() );

		// Set background level to TRANSLUCENT (see Geometry2DDataVtkMapper3D)
		m_Reslicer->SetBackgroundLevel( -32768 );

		// Calculate the actual bounds of the transformed plane clipped by the
		// dataset bounding box; this is required for drawing the texture at the
		// correct position during 3D mapping.
		boundsInitialized = this->CalculateClippedPlaneBounds(
			m_WorldGeometry->GetReferenceGeometry(), planeGeometry, sliceBounds );

	}
	else
	{
		// Do we have an AbstractTransformGeometry?
		// This is the case for AbstractTransformGeometry's (e.g. a thin-plate-spline transform)
		const mitk::AbstractTransformGeometry* abstractGeometry =
			dynamic_cast< const AbstractTransformGeometry * >(m_WorldGeometry);

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

			m_Reslicer->SetResliceTransform( composedResliceTransform );
			composedResliceTransform->UnRegister( NULL ); // decrease RC

			// Set background level to BLACK instead of translucent, to avoid
			// boundary artifacts (see Geometry2DDataVtkMapper3D)
			m_Reslicer->SetBackgroundLevel( -1023 );
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
		int interpolationMode = VTK_RESLICE_NEAREST;
		if ( m_ResliceInterpolationProperty != NULL )
		{
			interpolationMode = m_ResliceInterpolationProperty->GetInterpolation();
		}

		switch ( interpolationMode )
		{
		case VTK_RESLICE_NEAREST:
			m_Reslicer->SetInterpolationModeToNearestNeighbor();
			break;
		case VTK_RESLICE_LINEAR:
			m_Reslicer->SetInterpolationModeToLinear();
			break;
		case VTK_RESLICE_CUBIC:
			m_Reslicer->SetInterpolationModeToCubic();
			break;
		}
	}
	else
	{
		m_Reslicer->SetInterpolationModeToNearestNeighbor();
	}
	//### end set reslice interpolation

	
	vtkImageChangeInformation * unitSpacingImageFilter = vtkImageChangeInformation::New() ;
	unitSpacingImageFilter->ReleaseDataFlagOn();
	//for some strange reason you have to crop the outputimage to factor 3 in z-spacing (wtf)
	unitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 3.0 );
	unitSpacingImageFilter->SetInput( inputData );

	m_Reslicer->SetInput(unitSpacingImageFilter->GetOutput() );

	//number of pixels per mm in x- and y-direction of the resampled
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
	vnl2vtk( bottom.GetVnlVector(), cosines + 3 );//fill next 3 elements
	// normal of the plane
	vnl2vtk( normal.GetVnlVector(), cosines + 6 );//fill the last 3 elements

	m_Reslicer->SetResliceAxesDirectionCosines( cosines );

	int xMin, xMax, yMin, yMax;
	if ( boundsInitialized )
	{
		// Calculate output extent (integer values)
		xMin = static_cast< int >( sliceBounds[0] / mmPerPixel[0] ); //+ 0.5 );
		xMax = static_cast< int >( sliceBounds[1] / mmPerPixel[0] ); //+ 0.5 );
		yMin = static_cast< int >( sliceBounds[2] / mmPerPixel[1] ); //+ 0.5 );
		yMax = static_cast< int >( sliceBounds[3] / mmPerPixel[1] ); //+ 0.5 ); 
	}
	else
	{
		// If no reference geometry is available, we also don't know about the
		// maximum plane size;
		xMin = yMin = 0;
		xMax = static_cast< int >( extent[0]
		- pixelsPerMM[0] ); //+ 0.5 );
		yMax = static_cast< int >( extent[1]
		- pixelsPerMM[1] ); //+ 0.5 );
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

	if(m_ThickSlicesMode > 0)
	{
		dataZSpacing = 1.0 / normInIndex.GetNorm();
		m_Reslicer->SetOutputDimensionality( 3 );
		m_Reslicer->SetOutputExtent( xMin, xMax-1, yMin, yMax-1, -m_ThickSlicesNum, 0+m_ThickSlicesNum );
	}
	else
	{
		m_Reslicer->SetOutputDimensionality( 2 );
		m_Reslicer->SetOutputExtent( xMin, xMax-1, yMin, yMax-1, 0, 0 );
	}


	m_Reslicer->SetOutputOrigin( 0.0, 0.0, 0.0 );
	m_Reslicer->SetOutputSpacing( mmPerPixel[0], mmPerPixel[1], dataZSpacing );


	vtkImageData* reslicedImage;
	vtkMitkThickSlicesFilter* tsFilter;
	


	if(m_ThickSlicesMode>0)
	{
	    tsFilter->SetThickSliceMode( m_ThickSlicesMode-1 );
		tsFilter->SetInput( m_Reslicer->GetOutput() );
		tsFilter->Modified();
		tsFilter->Update();
		reslicedImage = tsFilter->GetOutput();
	}
	else
	{
	   m_Reslicer->Modified();
	   m_Reslicer->Update();
	   reslicedImage = m_Reslicer->GetOutput();
	}
	

	if((reslicedImage == NULL) || (reslicedImage->GetDataDimension() < 1))
	{
		itkWarningMacro(<<"Reslicer returned empty image");
		return;
	}


	mitk::Image::Pointer resultImage = this->GetOutput();

	//initialize resultimage with the specs of the vtkImageData object returned from vtkImageReslice
	resultImage->Initialize(reslicedImage);

	////transfer the voxel data
	resultImage->SetVolume(reslicedImage->GetScalarPointer());	

	//set the geometry from current worldgeometry for the reusultimage
	AffineGeometryFrame3D::Pointer originalGeometryAGF = m_WorldGeometry->Clone();
	Geometry2D::Pointer originalGeometry = dynamic_cast<Geometry2D*>( originalGeometryAGF.GetPointer() );
	originalGeometry->ChangeImageGeometryConsideringOriginOffset(true);
    resultImage->SetGeometry( originalGeometry );

	resultImage->GetGeometry()->TransferItkToVtkTransform();

	
	resultImage->GetGeometry()->Modified();
}


void mitk::ExtractDirectedPlaneImageFilter::GenerateOutputInformation()
{
	Superclass::GenerateOutputInformation();
}


bool mitk::ExtractDirectedPlaneImageFilter
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

bool mitk::ExtractDirectedPlaneImageFilter
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
