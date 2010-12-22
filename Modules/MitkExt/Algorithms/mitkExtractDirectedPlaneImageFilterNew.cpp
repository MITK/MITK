/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-12-02 14:54:03 +0100 (Do, 2 Dec 2010) $
Version:   $Revision: 21147 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkExtractDirectedPlaneImageFilterNew.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "itkImageRegionIterator.h"

#include "itkResampleImageFilter.h"
#include <itkCastImageFilter.h>
//#include "itkGaussianInterpolateImageFunction.h"

mitk::ExtractDirectedPlaneImageFilterNew::ExtractDirectedPlaneImageFilterNew(void)
:m_CurrentWorldGeometry2D(NULL)
{
}

mitk::ExtractDirectedPlaneImageFilterNew::~ExtractDirectedPlaneImageFilterNew(void)
{
}

void mitk::ExtractDirectedPlaneImageFilterNew::GenerateData(){

	mitk::Image::ConstPointer inputImage = ImageToImageFilter::GetInput(0);

	if ( !inputImage )
	{
		MITK_ERROR << "mitk::ExExtractDirectedPlaneImageFilterNew: No input available. Please set the input!" << std::endl;
		itkExceptionMacro("mitk::ExExtractDirectedPlaneImageFilterNew: No input available. Please set the input!");
		return;
	}

	m_ImageGeometry = inputImage->GetGeometry();

	//If no timestep is set, the lowest given will be selected
	const mitk::TimeSlicedGeometry* inputTimeGeometry = this->GetInput()->GetTimeSlicedGeometry();
	if ( !m_ActualInputTimestep ) 
	{
		ScalarType time = m_CurrentWorldGeometry2D->GetTimeBounds()[0];
		if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
		{
			m_ActualInputTimestep = inputTimeGeometry->MSToTimeStep( time );
		}
	}

	if ( inputImage->GetDimension() > 4 || inputImage->GetDimension() < 2)
	{
		MITK_ERROR << "mitk::ExExtractDirectedPlaneImageFilterNew:GenerateData works only with 3D and 3D+t images, sorry." << std::endl;
		itkExceptionMacro("mitk::ExExtractDirectedPlaneImageFilterNew works only with 3D and 3D+t images, sorry.");
		return;
	}
	else if ( inputImage->GetDimension() == 4 )
	{
		mitk::ImageTimeSelector::Pointer timeselector = mitk::ImageTimeSelector::New();
		timeselector->SetInput( inputImage );
		timeselector->SetTimeNr( m_ActualInputTimestep );
		timeselector->UpdateLargestPossibleRegion();
		inputImage = timeselector->GetOutput();
	}
	else if ( inputImage->GetDimension() == 2)
	{
		mitk::Image::Pointer resultImage = ImageToImageFilter::GetOutput();
		resultImage = const_cast<mitk::Image*>( inputImage.GetPointer() );
		ImageToImageFilter::SetNthOutput( 0, resultImage);
		return;
	}

	if ( !m_CurrentWorldGeometry2D )
	{
		MITK_ERROR<< "mitk::ExExtractDirectedPlaneImageFilterNew::GenerateData has no CurrentWorldGeometry2D set" << std::endl;
		return;
	}	

	AccessFixedDimensionByItk( inputImage, ItkSliceExtraction, 3 );

	/*AccessFixedDimensionByItk(
		inputImage,
            InternalReorientImagePlane, 3);*/


}//Generate Data


void mitk::ExtractDirectedPlaneImageFilterNew::GenerateOutputInformation () 
{
	Superclass::GenerateOutputInformation();
}



/*
 * The desired slice is extracted by filling the image`s corresponding pixel values in an empty 2 dimensional itk::Image
 * Therefor the itk image`s extent in pixel (in each direction) is doubled and its spacing (also in each direction) is divided by two
 * (shannon theorem).
 */
template<typename TPixel, unsigned int VImageDimension>
void mitk::ExtractDirectedPlaneImageFilterNew::ItkSliceExtraction (itk::Image<TPixel, VImageDimension>* inputImage)
{
	typedef itk::Image<TPixel, VImageDimension> InputImageType;
	typedef itk::Image<TPixel, VImageDimension-1> SliceImageType;

	typedef itk::ImageRegionConstIterator< SliceImageType > SliceIterator;

	//Creating an itk::Image that represents the sampled slice
	typename SliceImageType::Pointer resultSlice = SliceImageType::New();

	//Defining the origin which is represented by the index
	SliceImageType::IndexType start;

	start[0] = 0;
	start[1] = 0;

	//Determine the spacing and the extent of the resulting slice (note the the spacing is divided by two)
	Point3D origin = m_CurrentWorldGeometry2D->GetOrigin();
	Vector3D right = m_CurrentWorldGeometry2D->GetAxisVector(0);
	Vector3D bottom = m_CurrentWorldGeometry2D->GetAxisVector(1);
	Vector3D bottomInIndex;
	Vector3D rightInIndex;

	m_ImageGeometry->WorldToIndex(origin, right, rightInIndex);
	m_ImageGeometry->WorldToIndex(origin, bottom, bottomInIndex);

	Vector2D extentInPixel;
	extentInPixel[0] = rightInIndex.GetNorm();
	extentInPixel[1] = bottomInIndex.GetNorm();

	Vector2D extentInMM;
	extentInMM[0] = m_CurrentWorldGeometry2D->GetExtentInMM(0);
	extentInMM[1] = m_CurrentWorldGeometry2D->GetExtentInMM(1);	
	//extentInMM *= 2;

	//Test
	double maxExtent = sqrt(extentInMM[0]*extentInMM[0]+extentInMM[1]*extentInMM[1]);
	unsigned int xTranlation = (maxExtent-extentInMM[0]);
	unsigned int yTranlation = (maxExtent-extentInMM[1]);
	//Test ende

	/*mitk::ScalarType newPixelSpacing[3];
	newPixelSpacing[0] = extentInMM[0] / extentInPixel[0];
	newPixelSpacing[1] = extentInMM[1] / extentInPixel[1];*/
	//newPixelSpacing[2] = 1;

	Vector3D newPixelSpacing = m_CurrentWorldGeometry2D->GetSpacing();
	double minSpacing = newPixelSpacing[0];
	for (int i = 1; i < newPixelSpacing.Size(); i++)
	{
		if (newPixelSpacing[i] < minSpacing )
		{
			minSpacing = newPixelSpacing[i];
		}
	}

	newPixelSpacing[0] = 0.5*minSpacing;
	newPixelSpacing[1] = 0.5*minSpacing;
	newPixelSpacing[2] = 0.5*minSpacing;

	float pixelSpacing[2];
	pixelSpacing[0] =  newPixelSpacing[0];
	pixelSpacing[1] =  newPixelSpacing[1];

	SliceImageType::SizeType size;
	/*size[0] = extentInMM[0]/newPixelSpacing[0];
	size[1] = 2*extentInMM[1]/newPixelSpacing[1];*/
	//Test
	size[0] = (maxExtent+xTranlation)/newPixelSpacing[0];
	size[1] = (maxExtent+yTranlation)/newPixelSpacing[1];

	//Creating an ImageRegion Object
	SliceImageType::RegionType region;

	region.SetSize( size );
	region.SetIndex( start );

	//Defining the image`s extent and origin by passing the region to it and allocating memory for it
	resultSlice->SetRegions( region );
	resultSlice->SetSpacing( pixelSpacing );
	resultSlice->Allocate();

	/*
	 * Here we create an new geometry so that the transformations are calculated correctly (our resulting slice has a different bounding box and spacing)
	 * The original current worldgeometry must be cloned because we have to keep the directions of the axis vector which represents the rotation
     */
	//Test
	right.Normalize();
	bottom.Normalize();
	/*unsigned int xTranlation = (maxExtent-extentInMM[0]);
	unsigned int yTranlation = (maxExtent-extentInMM[0]);*/
	origin[0] -= xTranlation*right[0]+yTranlation*bottom[0];
	origin[1] -= xTranlation*right[1]+yTranlation*bottom[1];
	origin[2] -= xTranlation*right[2]+yTranlation*bottom[2];
	//Test Ende
	mitk::Geometry3D::Pointer newSliceGeometryTest = dynamic_cast<Geometry3D*>(m_CurrentWorldGeometry2D->Clone().GetPointer());
	//Test
	newSliceGeometryTest->SetOrigin(origin);
	//Test Ende
	ScalarType bounds[6]={0, size[0], 0, size[1], 0, 1};
	newSliceGeometryTest->SetBounds(bounds);
	newSliceGeometryTest->SetSpacing(newPixelSpacing);
	newSliceGeometryTest->Modified();

	/* 
	 * Now we iterate over the recently created slice.
	 * For each slice - pixel we check whether there is an according
	 * pixel in the input - image which can be set in the slice.
	 * In this way a slice is sampled out of the input - image regrading to the given PlaneGeometry
	 */
	Point3D currentSliceIndexPointIn3D;
	Point3D currentImageWorldPointIn3D;

	InputImageType::IndexType inputIndex;

	SliceIterator sliceIterator ( resultSlice, resultSlice->GetLargestPossibleRegion() );
	sliceIterator.GoToBegin();

	//std::fstream extractorDebugFile("C:/Users/fetzer/Desktop/SurfaceInterpolation/extractPoints.txt");

	while ( !sliceIterator.IsAtEnd() )
	{
		/*
		 * Here we add 0.5 to to assure that the indices are correctly transformed.
		 * (Because of the 0.5er Bug)
		 */
		currentSliceIndexPointIn3D[0] = sliceIterator.GetIndex()[0]+0.5;
		currentSliceIndexPointIn3D[1] = sliceIterator.GetIndex()[1]+0.5;
		currentSliceIndexPointIn3D[2] = 0;

		newSliceGeometryTest->IndexToWorld( currentSliceIndexPointIn3D, currentImageWorldPointIn3D );

		m_ImageGeometry->WorldToIndex( currentImageWorldPointIn3D, inputIndex);

		if ( m_ImageGeometry->IsIndexInside( inputIndex ))
		{
		SliceImageType::IndexType index = sliceIterator.GetIndex();

		resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );

		}

		//For the purpose of debug
		//if( debugCounter%100 == 0)
		//{
		 //extractorFile<<"[ "<<inputIndex[0]<<", "<<inputIndex[1]<<", "<<inputIndex[2]<<" ]"<<std::endl;
		//}
		// ++debugCounter;
		++sliceIterator;
		//columns++;

	}
	//extractorFile.close();

	Image::Pointer resultImage = ImageToImageFilter::GetOutput();
	GrabItkImageMemory(resultSlice, resultImage, NULL, false);
	resultImage->SetClonedGeometry(newSliceGeometryTest);
}


 //template < typename TPixel, unsigned int VImageDimension >
 //void mitk::ExtractDirectedPlaneImageFilterNew::InternalReorientImagePlane(
 //         const itk::Image< TPixel, VImageDimension > *image )
 // {

 //   MITK_INFO << "InternalReorientImagePlane() start";

 //   typedef itk::Image< TPixel, VImageDimension > ImageType;
 //   typedef itk::Image< float, VImageDimension > FloatImageType;

 //   typedef itk::ResampleImageFilter<ImageType, FloatImageType, double> ResamplerType;
 //   typename ResamplerType::Pointer resampler = ResamplerType::New();

	//mitk::Geometry3D* temp = const_cast<mitk::Geometry3D*>(m_CurrentWorldGeometry2D);
 //   mitk::PlaneGeometry* planegeo = dynamic_cast<mitk::PlaneGeometry*>(temp);

 //   float upsamp = 1 /*m_UpsamplingFactor*/;
 //  float gausssigma = 0 /*m_GaussianSigma*/;

 //   // Spacing
 //   typename ResamplerType::SpacingType spacing = planegeo->GetSpacing();
 //   spacing[0] = image->GetSpacing()[0] / upsamp;
 //   spacing[1] = image->GetSpacing()[1] / upsamp;
 //   spacing[2] = image->GetSpacing()[2];
 //   resampler->SetOutputSpacing( spacing );

 //   // Size
 //   typename ResamplerType::SizeType size;
 //   size[0] = planegeo->GetParametricExtentInMM(0) / spacing[0];
 //   size[1] = planegeo->GetParametricExtentInMM(1) / spacing[1];
 //   size[2] = 1;
 //   resampler->SetSize( size );

 //   // Origin
 //   typename mitk::Point3D orig = planegeo->GetOrigin();
 //   typename mitk::Point3D corrorig;
 //   m_CurrentWorldGeometry2D->WorldToIndex(orig,corrorig);
 //   corrorig[0] += 0.5/upsamp;
 //   corrorig[1] += 0.5/upsamp;
 //   corrorig[2] += 0;
 //   m_CurrentWorldGeometry2D->IndexToWorld(corrorig,corrorig);
 //   resampler->SetOutputOrigin(corrorig );

 //   // Direction
 //   typename ResamplerType::DirectionType direction;
 //   typename mitk::AffineTransform3D::MatrixType matrix = planegeo->GetIndexToWorldTransform()->GetMatrix();
 //   for(int c=0; c<matrix.ColumnDimensions; c++)
 //   {
 //     double sum = 0;
 //     for(int r=0; r<matrix.RowDimensions; r++)
 //     {
 //       sum += matrix(r,c)*matrix(r,c);
 //     }
 //     for(int r=0; r<matrix.RowDimensions; r++)
 //     {
 //       direction(r,c) = matrix(r,c)/sqrt(sum);
 //     }
 //   }
 //   resampler->SetOutputDirection( direction );

 //   // Gaussian interpolation
 //  /* if(gausssigma != 0)
 //   {
 //     double sigma[3];
 //     for( unsigned int d = 0; d < 3; d++ )
 //     {
 //       sigma[d] = gausssigma * image->GetSpacing()[d];
 //     }
 //     double alpha = 2.0;

 //     typedef itk::GaussianInterpolateImageFunction<ImageType, double>
 //         GaussianInterpolatorType;

 //     typename GaussianInterpolatorType::Pointer interpolator
 //         = GaussianInterpolatorType::New();

 //     interpolator->SetInputImage( image );
 //     interpolator->SetParameters( sigma, alpha );

 //     resampler->SetInterpolator( interpolator );
 //   }
 //   else
 //   {*/
 //     //      typedef typename itk::BSplineInterpolateImageFunction<ImageType, double>
 //     //          InterpolatorType;
 //     typedef typename itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;

 //     typename InterpolatorType::Pointer interpolator
 //         = InterpolatorType::New();

 //     interpolator->SetInputImage( image );

 //     resampler->SetInterpolator( interpolator );

 //  /* }*/

 //   // Other resampling options
 //   resampler->SetInput( image );
 //   resampler->SetDefaultPixelValue(0);

 //   MITK_INFO << "Resampling requested image plane ... ";
 //   resampler->Update();
 //   MITK_INFO << " ... done";

	//mitk::Image::Pointer resultImage = this->GetOutput();
	//GrabItkImageMemory(resampler->GetOutput(), resultImage, NULL, false);
	//

 // /*  if(additionalIndex < 0)
 //   {*/
 //     /*this->m_InternalImage = mitk::Image::New();
 //     this->m_InternalImage->InitializeByItk( resampler->GetOutput() );
 //     this->m_InternalImage->SetVolume( resampler->GetOutput()->GetBufferPointer() );*/
 //  /* }
 //   else
 //   {
 //     unsigned int myIndex = additionalIndex;
 //     this->m_InternalAdditionalResamplingImages.push_back(mitk::Image::New());
 //     this->m_InternalAdditionalResamplingImages[myIndex]->InitializeByItk( resampler->GetOutput() );
 //     this->m_InternalAdditionalResamplingImages[myIndex]->SetVolume( resampler->GetOutput()->GetBufferPointer() );
 //   }*/

 // }


///**TEST** May ba a little bit more efficient/
	//right.Normalize();
	//bottom.Normalize();
	//Point3D currentImagePointIn3D = origin /*+ bottom*newPixelSpacing*/;
	//unsigned int columns ( 0 );
	/**ENDE**/


/****TEST***/

		//SliceImageType::IndexType index = sliceIterator.GetIndex();

		//if ( columns == (extentInPixel[0]) )
		//{
		//	//If we are at the end of a row, then we have to go to the beginning of the next row 
		//	currentImagePointIn3D = origin;
		//	currentImagePointIn3D += newPixelSpacing[1]*bottom*index[1];
		//	columns = 0;
		//	m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
		//}
		//else
		//{
		//	//
		//	if ( columns != 0 )
		//	{
		//		currentImagePointIn3D += newPixelSpacing[0]*right; 
		//	}
		//	m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
		//}

		//if ( m_ImageGeometry->IsIndexInside( inputIndex ))
		//{
		//	resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );
		//}
		//else if (currentImagePointIn3D == origin)
		//{
		//	Point3D temp;
		//	temp[0] = bottom[0]*newPixelSpacing[0]*0.5;
		//	temp[1] = bottom[1]*newPixelSpacing[1]*0.5;
		//	temp[2] = bottom[2]*newPixelSpacing[2]*0.5;
		//	origin[0] += temp[0];
		//	origin[1] += temp[1];
		//	origin[2] += temp[2];
		//	currentImagePointIn3D = origin;
		//	m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
		//	if ( m_ImageGeometry->IsIndexInside( inputIndex ))
		//	{
		//		resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );
		//	}
		//}

		/****TEST ENDE****/
