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
#include <mitkRotationOperation.h>
#include <mitkInteractionConst.h>

#include <ctime> 
#include <cstdlib>


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


/*these are the deviations calculated by the function CalcTestFailureDeviation (see for details)*/
#define Testfailure_Deviation_Mean_128 0.853842

#define Testfailure_Deviation_Volume_128 0.145184
#define Testfailure_Deviation_Diameter_128 1.5625



#define Testfailure_Deviation_Mean_256 0.397693

#define Testfailure_Deviation_Volume_256 0.0141357
#define Testfailure_Deviation_Diameter_256 0.78125



#define Testfailure_Deviation_Mean_512 0.205277

#define Testfailure_Deviation_Volume_512 0.01993
#define Testfailure_Deviation_Diameter_512 0.390625



class mitkExtractSliceFilterTestClass{

public:


	static void TestSlice(mitk::PlaneGeometry* planeGeometry, std::string testname)
	{
		TestPlane = planeGeometry;
		TestName = testname;
		
		float centerCoordValue = TestvolumeSize / 2.0;
		float center[3] = {centerCoordValue, centerCoordValue, centerCoordValue};
		mitk::Point3D centerIndex(center);

		double radius = TestvolumeSize / 4.0;
		if(TestPlane->Distance(centerIndex) >= radius ) return;//outside sphere
		


		//feed ExtractSliceFilter
		mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();

		slicer->SetInput(TestVolume);
		slicer->SetWorldGeometry(TestPlane);
		slicer->Update();

		MITK_TEST_CONDITION_REQUIRED(slicer->GetOutput() != NULL, " Testing resliced Image returned");

		mitk::Image::Pointer reslicedImage = slicer->GetOutput();

		AccessFixedDimensionByItk(reslicedImage, TestSphereRadiusByItk, 2);
		AccessFixedDimensionByItk(reslicedImage, TestSphereAreaByItk, 2);



		

		MITK_INFO << TestName << ">>> " << "planeDistanceToSphereCenter: " << testResults.planeDistanceToSphereCenter;
		MITK_INFO << "area in pixels: " << testResults.areaInPixel << " <-> area in mm: " << testResults.areaCalculated << " = " << testResults.percentageAreaCalcToPixel << "%";
		
		MITK_INFO << "calculated diameter: " << testResults.diameterCalculated << " <-> diameter in mm: " << testResults.diameterInMM << " <-> diameter in pixel: " << testResults.diameterInPixel << " = " << testResults.percentageRadiusToPixel << "%";
		
	}


	template<typename TPixel, unsigned int VImageDimension>
	static void TestSphereRadiusByItk (itk::Image<TPixel, VImageDimension>* inputImage)
	{
		typedef itk::Image<TPixel, VImageDimension> InputImageType;
		

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
				typename InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexX[0];
				markIndex[1] = currentIndexX[1];


				indicesX.push_back(markIndex);
			}
			else if( inputImage->GetPixel(currentIndexX) < lastValueX)
			{
				//mark the current index
				typename InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexX[0];
				markIndex[1] = currentIndexX[1] - 1;//value inside the sphere


				indicesX.push_back(markIndex);
			}

			if(inputImage->GetPixel(currentIndexY) > lastValueY)
			{
				//mark the current index
				typename InputImageType::IndexType markIndex;
				markIndex[0] = currentIndexY[0];
				markIndex[1] = currentIndexY[1];


				indicesY.push_back(markIndex);
			}
			else if( inputImage->GetPixel(currentIndexY) < lastValueY)
			{
				//mark the current index
				typename InputImageType::IndexType markIndex;
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
		typename InputImageType::IndexType centerX, centerY;

		centerX[0] = indicesX[0][0];
		centerX[1] = indicesX[0][1] + distanceToCenterX;
		//TODO think about implicit cast to int, this is not the real center of the image. which could be between two pixels

		//centerY[0] = indicesY[0][0] + distanceToCenterY;
		//centerY[1] = inidcesY[0][1];

		typename InputImageType::IndexType currentIndex(centerX);
		lastValueX = inputImage->GetPixel(currentIndex);

		int sumpixels = 0;

		std::vector<InputImageType::IndexType> diameterIndices;

		//move up
		while(currentIndex[1] < TestvolumeSize)
		{
			currentIndex[1] += 1;

			if( inputImage->GetPixel(currentIndex) != lastValueX)
			{ 
				typename InputImageType::IndexType markIndex;
				markIndex[0] = currentIndex[0];
				markIndex[1] = currentIndex[1] - 1; 
				
				diameterIndices.push_back(markIndex); 
				break;
			}

			sumpixels++;
			lastValueX = inputImage->GetPixel(currentIndex);
		}
		
		currentIndex[1] -= sumpixels; //move back to center to go in the other direction
		lastValueX = inputImage->GetPixel(currentIndex);

		//move down
		while(currentIndex[1] >= 0)
		{
			currentIndex[1] -= 1;

			if( inputImage->GetPixel(currentIndex) != lastValueX)
				{ 
				typename InputImageType::IndexType markIndex;
				markIndex[0] = currentIndex[0];
				markIndex[1] = currentIndex[1];//outside sphere because we want to calculate the distance from edge to edge 
				
				diameterIndices.push_back(markIndex); 
				break;
			}
			
			sumpixels++;
			lastValueX = inputImage->GetPixel(currentIndex);
		}

		

		/*
		*Now sumpixels should be the apromximate diameter of the circle. This checked with the calculate diameter from the plane transformation(math).
		*/
		
		mitk::Point3D volumeCenter;
		volumeCenter[0] = volumeCenter[1] = volumeCenter[2] = TestvolumeSize / 2.0; 

		
		double planeDistanceToSphereCenter = TestPlane->Distance(volumeCenter);


		double sphereRadius = TestvolumeSize/4.0;

		//calculate the radius of the circle cut from the sphere by the plane
		double diameter = 2.0 * std::sqrt(std::pow(sphereRadius, 2) - std::pow( planeDistanceToSphereCenter  , 2));

		double percentageRadiusToPixel = 100 /  diameter  * sumpixels;
		
		

		/*
		 *calculate the radius in mm by the both marks of the center line by using the world coordinates
		 */
		
		//get the points as 3D coordinates
		mitk::Vector3D diameterPointRight, diameterPointLeft;

		diameterPointRight[2] = diameterPointLeft[2] = 0.0;
		
		diameterPointLeft[0] = diameterIndices[0][0];
		diameterPointLeft[1] = diameterIndices[0][1];

		diameterPointRight[0] = diameterIndices[1][0];
		diameterPointRight[1] = diameterIndices[1][1];

		//transform to worldcoordinates
		TestVolume->GetGeometry()->IndexToWorld(diameterPointLeft, diameterPointLeft);
		TestVolume->GetGeometry()->IndexToWorld(diameterPointRight, diameterPointRight);

		//euklidian distance
		double diameterInMM = ( (diameterPointLeft * -1.0) + diameterPointRight).GetNorm();


		testResults.diameterInMM = diameterInMM;
		testResults.diameterCalculated = diameter;
		testResults.diameterInPixel = sumpixels;
		testResults.percentageRadiusToPixel = percentageRadiusToPixel;
		testResults.planeDistanceToSphereCenter = planeDistanceToSphereCenter;
	}





	/*brute force the area pixel by pixel*/
	template<typename TPixel, unsigned int VImageDimension>
	static void TestSphereAreaByItk (itk::Image<TPixel, VImageDimension>* inputImage)
	{
		typedef itk::Image<TPixel, VImageDimension> InputImageType;
		typedef itk::ImageRegionConstIterator< InputImageType > ImageIterator;


		ImageIterator imageIterator( inputImage, inputImage->GetLargestPossibleRegion() );
		imageIterator.GoToBegin();

		int sumPixelsInArea = 0;

		while( !imageIterator.IsAtEnd() )
		{
			if(inputImage->GetPixel(imageIterator.GetIndex()) == pixelValueSet) sumPixelsInArea++;
			++imageIterator;
		}
		
		mitk::Point3D volumeCenter;
		volumeCenter[0] = volumeCenter[1] = volumeCenter[2] = TestvolumeSize / 2.0; 

		
		double planeDistanceToSphereCenter = TestPlane->Distance(volumeCenter);

		double sphereRadius = TestvolumeSize/4.0;

		//calculate the radius of the circle cut from the sphere by the plane
		double radius = std::sqrt(std::pow(sphereRadius, 2) - std::pow( planeDistanceToSphereCenter  , 2));

		double areaInMM = 3.14159265358979 * std::pow(radius, 2);//i don't want to include math.h only for pi! 
		

		testResults.areaCalculated = areaInMM;
		testResults.areaInPixel = sumPixelsInArea;
		testResults.percentageAreaCalcToPixel = 100 / areaInMM * sumPixelsInArea;
	}



	static void TestPerPixel()
	{
		typedef itk::Image<unsigned short, 3> ImageType;

		typedef itk::ImageRegionConstIterator< ImageType > ImageIterator;

		ImageType::Pointer image = ImageType::New();

		ImageType::IndexType start;
		start[0] = start[1] = start[2] = 0;

		ImageType::SizeType size;
		size[0] = size[1] = size[2] = 32;

		ImageType::RegionType imgRegion;
		imgRegion.SetSize(size);
		imgRegion.SetIndex(start);

		image->SetRegions(imgRegion);
		image->SetSpacing(1.0);
		image->Allocate();

		//image->FillBuffer(0);

		ImageIterator imageIterator( image, image->GetLargestPossibleRegion() );
		imageIterator.GoToBegin();

		
		unsigned short pixelValue = 0;
		
		while ( !imageIterator.IsAtEnd() )
		{
			image->SetPixel(imageIterator.GetIndex(), pixelValue);
			++imageIterator;
			++pixelValue;
		}
		
		mitk::Image::Pointer imageInMitk = mitk::Image::New();;
		CastToMitkImage(image, imageInMitk);

		srand((unsigned)time(0));
		unsigned char x = rand() % 31 +1; // 1 .. 31 we don't want [0,0,0]
		unsigned char y = rand() % 31 +1;
		unsigned char z = rand() % 31 +1;

		mitk::Point3D intersectionPoint;
		intersectionPoint[0] = x;
		intersectionPoint[1] = y;
		intersectionPoint[2] = z;

		ImageType::IndexType intersectionIndex;
		intersectionIndex[0] = intersectionPoint[0];
		intersectionIndex[1] = intersectionPoint[1];
		intersectionIndex[2] = intersectionPoint[2];


		unsigned short valueAtIntersecPoint = image->GetPixel(intersectionIndex);

		MITK_INFO << "intersection point: " << intersectionPoint << " value at intersection: " << valueAtIntersecPoint;

		

		float spacingArray[3] = {1.0, 1.0, 1.0};
		mitk::Vector3D spacing(spacingArray);

		mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
		plane->InitializeStandardPlane(imageInMitk->GetGeometry(), mitk::PlaneGeometry::PlaneOrientation::Sagittal, 16, true, false);
		plane->SetImageGeometry(true);

		
		mitk::Vector3D rotationVector;
		rotationVector[0] = randFloat();
		rotationVector[1] = randFloat();
		rotationVector[2] = randFloat();

		MITK_INFO << "rotationVector: " << rotationVector;

		mitk::RotationOperation* op = new mitk::RotationOperation(mitk::OpROTATE, plane->GetCenter(), rotationVector, 30.0);
		plane->ExecuteOperation(op);
		delete op;

		mitk::Point3D initOrigin =  plane->GetOrigin();
		mitk::Vector3D translation;
		translation[0] = intersectionPoint[0] - initOrigin[0];
		translation[1] = intersectionPoint[1] - initOrigin[1];
		translation[2] = intersectionPoint[2] - initOrigin[2];
		MITK_INFO << "Point transform: " << translation;

		plane->SetOrigin(intersectionPoint);
		plane->Modified();


		
		

		

		
		
		MITK_INFO << "point is inside: " << plane->IsInside(intersectionPoint);
		
		mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
		slicer->SetInput(imageInMitk);
		slicer->SetWorldGeometry(plane);
		slicer->Update();

		mitk::Image::Pointer slice = slicer->GetOutput();
		

		//geometrically reconstruction of 2D point from transformed 3D Point
		mitk::Vector3D axis0 = plane->GetAxisVector(0);
		mitk::Vector3D axis1 = plane->GetAxisVector(1);
		axis0.Normalize();
		axis1.Normalize();
		
		mitk::Point3D idx;
		idx[0] = std::sqrt(std::pow(translation * axis0 / axis0.GetSquaredNorm(),2) + std::pow(translation.GetNorm(), 2));
		idx[1] = std::sqrt(std::pow(translation * axis1 / axis1.GetSquaredNorm(),2) + std::pow(translation.GetNorm(), 2));
		idx[2] = 0;

		MITK_INFO << idx;
		unsigned short valueAtSlice = slice->GetPixelValueByWorldCoordinate(idx);
		
		MITK_INFO << valueAtSlice;

		mitk::Index3D curr;
		curr[0] = curr[1] = 0;

		for( int i = 0; i < 32 ; ++i){
			for( int j = 0; j < 32; ++j){
				++curr[1];
				if(slice->GetPixelValueByIndex(curr) == valueAtIntersecPoint){
					MITK_INFO << valueAtIntersecPoint << " MATCHED mitk " << curr;
					goto br;
				}
			}
			curr[1] = 0;
			++curr[0];
		}
br:
		mitk::Point2D p0;
		p0[0] = curr[0];
		p0[1] = curr[1];

		plane->IndexToWorld(p0,p0);
		MITK_INFO << p0;

		typedef itk::Image<unsigned short, 2> Image2DType;

		Image2DType::Pointer img  = Image2DType::New();
		CastToItkImage(slice, img);
		typedef itk::ImageRegionConstIterator< Image2DType > Iterator2D;
		
		Iterator2D iter(img, img->GetLargestPossibleRegion());
		iter.GoToBegin();
		while( !iter.IsAtEnd() ){
		
			if(img->GetPixel(iter.GetIndex()) == valueAtIntersecPoint) MITK_INFO << valueAtIntersecPoint << " MATCHED itk " << iter.GetIndex();
			
			++iter;
		}
	}

	static float randFloat(){ return (((float)rand()+1.0) / ((float)RAND_MAX + 1.0)) + (((float)rand()+1.0) / ((float)RAND_MAX + 1.0)) / ((float)RAND_MAX + 1.0);}


	static void CreateTestVolume()
	{

	#ifdef CREATE_VOLUME

		
		//do sphere creation
		ItkVolumeGeneration();

		//save in file
		mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
		writer->SetInput(TestVolume);

		
		std::string file;/* = "C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\sphere_";
		file += TestvolumeSize;
		file += ".nrrd";*/

		std::ostringstream filename;
		filename << "C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\sphere_";
		filename << TestvolumeSize;
		filename << ".nrrd";

		file = filename.str();
		 
		writer->SetFileName(file);
		writer->Update();

	#endif


	#ifndef CREATE_VOLUME //read from file
		
		mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();

		std::string filename = locator->FindFile("sphere_512.nrrd.mhd", "Modules/ImageExtraction/Testing/Data");

		mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
		reader->SetFileName(filename);
	
		reader->Update();
		TestVolume = reader->GetOutput();

	#endif

		/* uncomment this if deviation should by processed*/
		//get the TestFailureDeviation in %
		AccessFixedDimensionByItk(TestVolume, CalcTestFailureDeviation, 3);
	}

	
	struct SliceProperties{
		double planeDistanceToSphereCenter;
		double diameterInMM;
		double diameterInPixel;
		double diameterCalculated;
		double percentageRadiusToPixel;
		double areaCalculated;
		double areaInPixel;
		double percentageAreaCalcToPixel;
	};


	static mitk::Image::Pointer TestVolume;
	static double TestvolumeSize;
	static mitk::PlaneGeometry::Pointer TestPlane;
	static std::string TestName;
	static unsigned char pixelValueSet;
	static SliceProperties testResults;
	static double TestFailureDeviation;



private:
	
	/*Generate a sphere with a radius of TestvolumeSize / 4.0 */
	static void ItkVolumeGeneration ()
	{
		typedef itk::Image<unsigned char, 3> TestVolumeType;

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

		sphereImage->FillBuffer(0);

		
		mitk::Vector3D center;
		center[0] = center[1] = center[2] = TestvolumeSize / 2.0;


		double radius = TestvolumeSize / 4.0;

		double pixelValue = pixelValueSet;

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


	



	template<typename TPixel, unsigned int VImageDimension>
	static void CalcTestFailureDeviation (itk::Image<TPixel, VImageDimension>* inputImage)
	{
		typedef itk::Image<TPixel, VImageDimension> InputImageType;
		typedef itk::ImageRegionConstIterator< InputImageType > ImageIterator;

		ImageIterator iterator(inputImage, inputImage->GetLargestPossibleRegion());
		iterator.GoToBegin();

		int volumeInPixel = 0;

		while( !iterator.IsAtEnd() )
		{
			if(inputImage->GetPixel(iterator.GetIndex()) == pixelValueSet) volumeInPixel++;
			++iterator;
		}

		double diameter  = TestvolumeSize / 2.0;
		double volumeCalculated = (1.0 / 6.0) * 3.14159265358979 * std::pow(diameter, 3);

		
		double volumeDeviation = std::abs( 100 - (100 / volumeCalculated * volumeInPixel) );


		typename InputImageType::IndexType index;
		index[0] = index[1] =  TestvolumeSize / 2.0;
		index[2] = 0;

		int sumpixels = 0;
		while (index[2] < TestvolumeSize )
		{
			if(inputImage->GetPixel(index) == pixelValueSet) sumpixels++;
			index[2] += 1;
		}
		
		
		double diameterDeviation = std::abs( 100 - (100 / diameter * sumpixels) );
		MITK_INFO << "volume deviation: " << volumeDeviation << " diameter deviation:" << diameterDeviation;
		mitkExtractSliceFilterTestClass::TestFailureDeviation = (volumeDeviation + diameterDeviation) / 2.0;
	}


};
/***************** #END class *********************************************************/





/********************** #BEGIN Instanciation of members **********************************************************/
mitk::Image::Pointer mitkExtractSliceFilterTestClass::TestVolume = mitk::Image::New();
double mitkExtractSliceFilterTestClass::TestvolumeSize = 512.0;
mitk::PlaneGeometry::Pointer mitkExtractSliceFilterTestClass::TestPlane = mitk::PlaneGeometry::New();
std::string mitkExtractSliceFilterTestClass::TestName = "";
unsigned char mitkExtractSliceFilterTestClass::pixelValueSet = 255;
mitkExtractSliceFilterTestClass::SliceProperties mitkExtractSliceFilterTestClass::testResults = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
double mitkExtractSliceFilterTestClass::TestFailureDeviation = 0.0;
/********************** #END Instanciation of members **********************************************************/





/********************** #BEGIN test main **********************************************************/
int mitkExtractSliceFilterTest(int argc, char* argv[])
{

	MITK_TEST_BEGIN("mitkExtractSliceFilterTest")


	mitkExtractSliceFilterTestClass::TestPerPixel();

	mitkExtractSliceFilterTestClass::CreateTestVolume();
	

	mitk::Vector3D spacing = mitkExtractSliceFilterTestClass::TestVolume->GetGeometry()->GetSpacing();

	double sphereCenter = mitkExtractSliceFilterTestClass::TestvolumeSize / 2.0;


	mitk::PlaneGeometry::Pointer geometryTransversal = mitk::PlaneGeometry::New();
	geometryTransversal->InitializeStandardPlane(256.0, 256.0, spacing, mitk::PlaneGeometry::PlaneOrientation::Transversal, sphereCenter, false, true);
	geometryTransversal->SetImageGeometry(true);

	mitkExtractSliceFilterTestClass::TestSlice(geometryTransversal, "Testing transversal plane");



	mitk::PlaneGeometry::Pointer geometrySagital = mitk::PlaneGeometry::New();
	geometrySagital->InitializeStandardPlane(256.0, 256.0, spacing, mitk::PlaneGeometry::PlaneOrientation::Sagittal, sphereCenter, true, false);
	geometrySagital->SetImageGeometry(true);

	mitkExtractSliceFilterTestClass::TestSlice(geometrySagital, "Testing sagittal plane");



	mitk::PlaneGeometry::Pointer geometrySagitalShifted = mitk::PlaneGeometry::New();
	geometrySagitalShifted->InitializeStandardPlane(256.0, 256.0, spacing, mitk::PlaneGeometry::PlaneOrientation::Sagittal, (sphereCenter - 14), true, false);
	geometrySagitalShifted->SetImageGeometry(true);

	mitkExtractSliceFilterTestClass::TestSlice(geometrySagitalShifted, "Testing sagittal plane shifted");



	mitk::PlaneGeometry::Pointer geometryCoronal = mitk::PlaneGeometry::New();
	geometryCoronal->InitializeStandardPlane(256.0, 256.0, spacing, mitk::PlaneGeometry::PlaneOrientation::Frontal, sphereCenter, true, false);
	geometryCoronal->SetImageGeometry(true);

	mitkExtractSliceFilterTestClass::TestSlice(geometryCoronal, "Testing coronal plane");
	



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
		slicer->SetWorldGeometry(geometryCoronal);

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