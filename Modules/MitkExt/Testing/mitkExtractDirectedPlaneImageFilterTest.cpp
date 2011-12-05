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

#include "mitkExtractDirectedPlaneImageFilter.h"
#include "mitkStandardFileLocations.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
 *  Tests for the class "mitkExtractDirectedPlaneImageFilter".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkExtractDirectedPlaneImageFilterTest(int /* argc */, char* /*argv*/[])
{
	// always start with this!
	MITK_TEST_BEGIN("mitkExtractDirectedPlaneImageFilter")


		mitk::ExtractDirectedPlaneImageFilter::Pointer extractor = mitk::ExtractDirectedPlaneImageFilter::New();
	MITK_TEST_CONDITION_REQUIRED(extractor.IsNotNull(),"Testing instantiation") 


		std::vector<ExtractionTesting::Testcase> allTests = ExtractionTesting::InitializeTestCases();

	for( int i = 0; i < allTests.size(); i++);{

		ExtractionTesting::Testcase testCase = allTest[i];

		ExtractionTesting::DoTesting(testCase);

		MITK_TEST_CONDITION(testCase.success, "Testcase #'" << testCase.number << " " << testCase.name); 
	}

	// always end with this!
	MITK_TEST_END()
}

class ExtractionTesting{

public:

	struct Testcase
	{
		int number;
		std::string name;
		std::string imageFilename;
		std::string referenceImageFilename;
		bool success;
		mitk::Geometry2D::Pointer (*GetPlane) (void);
	};

	static void DoTesting(Testcase &testcase)
	{
		mitk::Image::Pointer image = GetImageToTest(testcase.imageFilename);
		if ( image.IsNull){
			testcase.success = false;
			return;
		}

		mitk::Image::Pointer referenceImage = GetImageToTest(testcase.referenceImageFilename);
		if ( referenceImage.IsNull){
			testcase.success = false;
			return;
		}

		mitk::Geometry2D::Pointer directedGeometry2D = testcase.GetPlane();
		if(directedGeometry2D.IsNull){
			testcase.success = false;
		}

		//put testing here
	}


	static std::vector<testCase> InitializeTestCases()
	{
		int testcounter = 0;
		std::vector<testCase> tests=

		//#BEGIN setup TestCases

		{
			{
				++testcounter,
				"TestCoronal",
				"image.nrrd",
				"coronalReference.nrrd",
				false,
				&TestCoronal
			},
			{
				++testcounter,
				"TestSagital",
				"image.nrrd",
				"sagitalReference.nrrd",
				false,
				&TestSagital
			},
			{
				++testcounter,
				"TestCoronal",
				"image.nrrd",
				"coronalReference.nrrd",
				false,
				&TestCoronal
			},
			{
				++testcounter,
				"Test_u_Rotation",
				"image.nrrd",
				"uRotationReference.nrrd",
				false,
				&Test_u_Rotation
			},
			{
				++testcounter,
				"Test_v_Rotation",
				"image.nrrd",
				"vRotationReference.nrrd",
				false,
				&Test_v_Rotation
			},
			{
				++testcounter,
				"TestTwoDirectionalRation",
				"image.nrrd",
				"twoDirectionalRationReference.nrrd",
				false,
				&TestTwoDirectionalRotation
			},
			{
				++testcounter,
				"Test4D",
				"image.nrrd",
				"twoDirectionalRationReference.nrrd",
				false,
				&Test4D
			},
			{
				++testcounter,
				"Test2D",
				"coronalReference.nrrd",
				"coronalReference.nrrd",
				false,
				&Test2D
			},
			{
				++testcounter,
				"Test2D",
				NULL,
				NULL,
				false,
				&Test1D
			}

		};

		//#END setup TestCases

		return tests;
	}

protected:

	static mitk::Image::Pointer GetImageToTest(std::string filename){
		//retrieve referenceImage

		mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();

		const std::string filepath = locator->FindFile(filename, "Modules/MitkExt/Testing/Data");

		if (filepath.empty())
		{
			return NULL;
		}

//TODO read imge from file
		itk::FilenamesContainer file;
		file.push_back( filename );

		mitk::Image::Pointer image = mitk::Image::New();

		return image;
	}


	static mitk::Geometry2D::Pointer TestSagital()
	{
		

	}

	static mitk::Geometry2D::Pointer TestCoronal()
	{

	}

	static mitk::Geometry2D::Pointer TestTransversal()
	{

	}

	static mitk::Geometry2D::Pointer Test_u_Rotation()
	{

	}

	static mitk::Geometry2D::Pointer Test_v_Rotation()
	{

	}

	static mitk::Geometry2D::Pointer TestTwoDirectionalRotation()
	{

	}

	static mitk::Geometry2D::Pointer Test4DImage()
	{

	}

	static mitk::Geometry2D::Pointer Test2DImage()
	{

	}

	static mitk::Geometry2D::Pointer Test1DImage()
	{

	}

};

