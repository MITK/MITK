/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkExtractSliceFilter.h>
#include <mitkTestingMacros.h>
#include <mitkItkImageFileReader.h>
#include <itkImageRegionIterator.h>
#include <mitkImageCast.h>
#include <itkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkStandardFileLocations.h>
#include <mitkImageWriter.h>
#include <mitkITKImageImport.h>


#include <mitkGeometry3D.h>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkTexture.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlaneSource.h>





//#define CREATE_VOLUME
#define SHOW_SLICE_IN_RENDER_WINDOW

class mitkExtractSliceFilterTestClass{

public:


	static void TestSlice(mitk::PlaneGeometry* planeGeometry)
	{
		TestPlane = planeGeometry;
		
		
		float center[3] = {64.0, 64.0, 64.0};
		mitk::Point3D centerIndex(center);
		if(TestPlane->Distance(centerIndex) > 32.0 ) return;//outside sphere
		


		//feed ExtractSliceFilter
		mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();

		slicer->SetInput(TestVolume);
		slicer->SetWorldGeometry(TestPlane);
		slicer->Update();

		MITK_TEST_CONDITION_REQUIRED(slicer->GetOutput() != NULL, " Testing resliced Image returned");

		mitk::Image::Pointer reslicedImage = slicer->GetOutput();

		AccessFixedDimensionByItk(reslicedImage, TestSliceByItk, 2);
		
	}


	template<typename TPixel, unsigned int VImageDimension>
	static void TestSliceByItk (itk::Image<TPixel, VImageDimension>* inputImage)
	{
		typedef itk::Image<TPixel, VImageDimension> InputImageType;
		

		inputImage->Allocate();		


		//set the index to the middle of the image's edge at x and y axis
		InputImageType::IndexType currentIndexX;
		currentIndexX[0] = (int)(TestvolumeSize / 2.0);
		currentIndexX[1] = 0;

		InputImageType::IndexType currentIndexY;
		currentIndexY[0] = 0;
		currentIndexY[1] = (int)(TestvolumeSize / 2.0);


		//remember the last pixel value
		double lastValueX = inputImage->GetPixel(currentIndexX);
		double lastValueY = inputImage->GetPixel(currentIndexY);


		//storage for the index marks
		std::vector<InputImageType::IndexType> indicesX;
		std::vector<InputImageType::IndexType> indicesY;


		/*Get four indices on the edge of the circle*/
		while(currentIndexX[1] < TestvolumeSize && currentIndexX[0] < TestvolumeSize)
		{
			//move x direction
			currentIndexX[1] += 1;

			//move y direction
			currentIndexY[0] += 1;

			if(inputImage->GetPixel(currentIndexX) > lastValueX)
			{
				//mark the current index
				InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexX[0];
				markIndex[1] = currentIndexX[1];


				indicesX.push_back(markIndex);
			}
			else if( inputImage->GetPixel(currentIndexX) < lastValueX)
			{
				//mark the current index
				InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexX[0];
				markIndex[1] = currentIndexX[1] - 1;//value inside the sphere


				indicesX.push_back(markIndex);
			}

			if(inputImage->GetPixel(currentIndexY) > lastValueY)
			{
				//mark the current index
				InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexY[0];
				markIndex[1] = currentIndexY[1];


				indicesY.push_back(markIndex);
			}
			else if( inputImage->GetPixel(currentIndexY) < lastValueY)
			{
				//mark the current index
				InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexY[0];
				markIndex[1] = currentIndexY[1] - 1;//value inside the sphere


				indicesY.push_back(markIndex);
			}


			//found both marks?
			if(indicesX.size() == 2 && indicesY.size() == 2) break;
			
			//the new 'last' values
			lastValueX = inputImage->GetPixel(currentIndexX);
			lastValueY = inputImage->GetPixel(currentIndexY);
		}

		/*
		 *If we are here we found the four marks on the edge of the circle.
		 *For the case our plane is rotated and shifted, we have to calculate the center of the circle,
		 *else the center is the intersection of both straight lines between the marks.
		 *When we have the center, the diameter of the circle will be checked to the reference value(math!).
		 */

		//each distance from the first mark of each direction to the center of the straight line between the marks 
		double distanceToCenterX = std::abs(indicesX[0][1] - indicesX[1][1]) / 2.0;
		double distanceToCenterY = std::abs(indicesY[0][0] - indicesY[1][0]) / 2.0;


		//the center of the straight lines
		InputImageType::IndexType centerX, centerY;

		centerX[0] = indicesX[0][0];
		centerX[1] = indicesX[0][1] + distanceToCenterX;
		//TODO think about implicit cast to int, this is not the real center of the image. which could be between two pixels

		//centerY[0] = indicesY[0][0] + distanceToCenterY;
		//centerY[1] = inidcesY[0][1];

		InputImageType::IndexType currentIndex(centerX);
		lastValueX = inputImage->GetPixel(currentIndex);

		int sumpixels = 1;//center is the first pixel

		//move up
		while(currentIndex[1] < TestvolumeSize)
		{
			currentIndex[1] += 1;

			if( inputImage->GetPixel(currentIndex) != lastValueX) break;

			sumpixels++;
			lastValueX = inputImage->GetPixel(currentIndex);
		}
		
		currentIndex[1] -= sumpixels; //move back to center to go in the other direction
		lastValueX = inputImage->GetPixel(currentIndex);

		//move down
		while(currentIndex[1] >= 0)
		{
			currentIndex[1] -= 1;

			if( inputImage->GetPixel(currentIndex) != lastValueX) break;
			
			sumpixels++;
			lastValueX = inputImage->GetPixel(currentIndex);
		}

		/*
		*Now sumpixels should be the apromximate diameter of the circle. This checked with the calculate diameter from the plane transformation(math).
		*/
		
		mitk::Point3D volumeCenter;
		volumeCenter[0] = volumeCenter[1] = volumeCenter[2] = 64.0; 

		
		double planeDistanceToSphereCenter = TestPlane->Distance(volumeCenter);

		MITK_INFO << "planeDistanceToSphereCenter: " << planeDistanceToSphereCenter;

		double sphereRadius = TestvolumeSize/4.0;

		//calculate the radius of the circle cut from the sphere by the plane
		double diameter = 2.0 * std::sqrt(std::pow(sphereRadius, 2) - std::pow( planeDistanceToSphereCenter  , 2));

		MITK_INFO << "calculated diameter: " << diameter << " <-> diameter in pixel: " << sumpixels;
	}



	static void CreateTestVolume()
	{

	#ifdef CREATE_VOLUME

		
		//do sphere creation
		ItkVolumeGeneration();

		//save in file
		mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
		writer->SetInput(TestVolume);
		writer->SetFileName("C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\sphere_128_128_128.nrrd");
		writer->Update();

	#endif


	#ifndef CREATE_VOLUME //read from file
		
		mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
		const std::string filename = locator->FindFile("sphere_128_128_128.nrrd.mhd", "Modules/ImageExtraction/Testing/Data");

		mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
		reader->SetFileName(filename);
	
		reader->Update();
		TestVolume = reader->GetOutput();

	#endif

	}

	

	static mitk::Image::Pointer TestVolume;
	static const double TestvolumeSize;
	static mitk::PlaneGeometry::Pointer TestPlane;
	static std::string TestName;


private:

	static void ItkVolumeGeneration ()
	{
		typedef itk::Image<double, 3> TestVolumeType;

		typedef itk::ImageRegionConstIterator< TestVolumeType > ImageIterator;

		TestVolumeType::Pointer sphereImage = TestVolumeType::New();

		TestVolumeType::IndexType start;
		start[0] = start[1] = start[2] = 0;

		TestVolumeType::SizeType size;
		size[0] = size[1] = size[2] = TestvolumeSize;

		TestVolumeType::RegionType imgRegion;
		imgRegion.SetSize(size);
		imgRegion.SetIndex(start);

		sphereImage->SetRegions(imgRegion);
		sphereImage->SetSpacing(1.0);
		sphereImage->Allocate();

		sphereImage->FillBuffer(0.0);

		
		mitk::Vector3D center;
		center[0] = center[1] = center[2] = 64.0;


		double radius = 32.0;

		double pixelValue = 255.0;

		double distanceToCenter = 0.0;

		
		ImageIterator imageIterator( sphereImage, sphereImage->GetLargestPossibleRegion() );
		imageIterator.GoToBegin();

		
		mitk::Vector3D currentVoxelInIndex;
		
		while ( !imageIterator.IsAtEnd() )
		{
			currentVoxelInIndex[0] = imageIterator.GetIndex()[0];
			currentVoxelInIndex[1] = imageIterator.GetIndex()[1];
			currentVoxelInIndex[2] = imageIterator.GetIndex()[2];

			distanceToCenter = (center + ( currentVoxelInIndex * -1.0 )).GetNorm();

			//if distance to center is smaller then the radius of the sphere
			if( distanceToCenter < radius)
			{			
				sphereImage->SetPixel(imageIterator.GetIndex(), pixelValue);
			}

			++imageIterator;
		}

		CastToMitkImage(sphereImage, TestVolume);
	}



};

mitk::Image::Pointer mitkExtractSliceFilterTestClass::TestVolume = mitk::Image::New();
const double mitkExtractSliceFilterTestClass::TestvolumeSize = 128.0;
mitk::PlaneGeometry::Pointer mitkExtractSliceFilterTestClass::TestPlane = mitk::PlaneGeometry::New();
std::string mitkExtractSliceFilterTestClass::TestName = "";


int mitkExtractSliceFilterTest(int argc, char* argv[])
{

	MITK_TEST_BEGIN("mitkExtractSliceFilterTest")


	mitkExtractSliceFilterTestClass::CreateTestVolume();
	

	mitk::Vector3D spacing;
	spacing[0] = spacing[1] = spacing[2] = 1.0;

	mitk::PlaneGeometry::Pointer geometryTransversal = mitk::PlaneGeometry::New();
	
	geometryTransversal->InitializeStandardPlane(256.0, 256.0, spacing, mitk::PlaneGeometry::PlaneOrientation::Transversal, 50, false, true);
	geometryTransversal->SetImageGeometry(true);

	mitkExtractSliceFilterTestClass::TestName = "Testing transversal plane";
	mitkExtractSliceFilterTestClass::TestSlice(geometryTransversal);
	



#ifdef SHOW_SLICE_IN_RENDER_WINDOW
/*************** #BEGIN vtk render code ************************/
	
//set reslicer for renderwindow
	/*mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();

	std::string filename =  "C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\Pic3D.nrrd";
	reader->SetFileName(filename);
	
	reader->Update();

	mitk::Image::Pointer pic = reader->GetOutput();*/
	mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
	slicer->SetInput(mitkExtractSliceFilterTestClass::TestVolume);
	slicer->SetWorldGeometry(geometryTransversal);

	slicer->Update();


//set vtk renderwindow
	vtkSmartPointer<vtkPlaneSource> vtkPlane = vtkSmartPointer<vtkPlaneSource>::New();
	vtkPlane->SetOrigin(0.0, 0.0, 0.0);
	//These two points define the axes of the plane in combination with the origin.
	//Point 1 is the x-axis and point 2 the y-axis.
	//Each plane is transformed according to the view (transversal, coronal and saggital) afterwards.
	vtkPlane->SetPoint1(1.0, 0.0, 0.0); //P1: (xMax, yMin, depth)
	vtkPlane->SetPoint2(0.0, 1.0, 0.0); //P2: (xMin, yMax, depth)
	//these are not the correct values for all slices, only a square plane by now

	vtkSmartPointer<vtkPolyDataMapper> imageMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	imageMapper->SetInputConnection(vtkPlane->GetOutputPort());

	
	vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();

	//built a default lookuptable
	lookupTable->SetRampToLinear();
	lookupTable->SetSaturationRange( 0.0, 0.0 );
	lookupTable->SetHueRange( 0.0, 0.0 );
	lookupTable->SetValueRange( 0.0, 1.0 );
	lookupTable->Build();
	//map all black values to transparent
	lookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	lookupTable->SetRange(-255.0, 255.0);
	//lookupTable->SetRange(-1022.0, 1184.0);//pic3D range

	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();


	texture->SetInput(slicer->GetOutput()->GetVtkImageData());
	
	texture->SetLookupTable(lookupTable);

	texture->SetMapColorScalarsThroughLookupTable(true);

	vtkSmartPointer<vtkActor> imageActor = vtkSmartPointer<vtkActor>::New();
	imageActor->SetMapper(imageMapper);
	imageActor->SetTexture(texture);
	

	// Setup renderers
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(imageActor);

	// Setup render window
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// Setup render window interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// Render and start interaction
	renderWindowInteractor->SetRenderWindow(renderWindow);
	//renderer->AddViewProp(imageActor);


	renderWindow->Render();
	renderWindowInteractor->Start();
	// always end with this!
/*********************** #END vtk render code ****************************************/
#endif //SHOW_SLICE_IN_RENDER_WINDOW


	MITK_TEST_END()
}