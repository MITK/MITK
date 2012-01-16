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
#include "mitkTestingMacros.h"

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

#include <mitkItkImageFileReader.h>

#include <mitkRotationOperation.h>
#include "mitkInteractionConst.h"


static bool TestSlice(mitk::Image* imageVolume, mitk::PlaneGeometry* planeGeometry, std::string referenceFileName ){

	bool slicesAreEqual = false;

	mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
	reader->SetFileName(referenceFileName);	
	reader->Update();

	mitk::Image::Pointer referenceImage = reader->GetOutput();
	MITK_TEST_CONDITION(referenceImage.IsNotNull(), "Reference image read");


	//feed ExtractSliceFilter
	mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();

	slicer->SetInput(imageVolume);
	slicer->SetWorldGeometry(planeGeometry);
	slicer->Update();

	MITK_TEST_CONDITION_REQUIRED(slicer->GetOutput() != NULL, " Testing resliced Image returned");

	mitk::Image::Pointer reslicedImage = slicer->GetOutput();
	

	//the current index we are comparing the images at
	mitk::Point3D currentIndex = planeGeometry->GetOrigin();
	imageVolume->GetGeometry()->WorldToIndex(currentIndex, currentIndex);


	//calculate the direction vectors for iteration
	mitk::Vector3D right, bottom;
	right = planeGeometry->GetAxisVector(0);
	bottom = planeGeometry->GetAxisVector(1);
	imageVolume->GetGeometry()->WorldToIndex(right, right);
	imageVolume->GetGeometry()->WorldToIndex(bottom, bottom);
	right.Normalize();
	bottom.Normalize();


	double epsilon = 0.05; //TODO think about the value

	//loop through colums
	while( imageVolume->GetGeometry()->IsIndexInside(currentIndex) ){
		//loop through rows
		while( imageVolume->GetGeometry()->IsIndexInside(currentIndex) ){

			mitk::Index3D idx;
			idx.CopyWithCast(currentIndex);

			if((referenceImage->GetPixelValueByIndex( idx ) / reslicedImage->GetPixelValueByIndex( idx ) ) < epsilon){

				slicesAreEqual = true;
				currentIndex += right;

			}
			else{
				slicesAreEqual = false;
				break;
			}
		}
		
		currentIndex += bottom;
	}

	
	return slicesAreEqual;
}



int mitkExtractSliceFilterTest(int argc, char* argv[])
{

	MITK_TEST_BEGIN("mitkExtractSliceFilterTest")


	mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();

	std::string filename =  "C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\Pic3D.nrrd";
	reader->SetFileName(filename);
	
	reader->Update();

	mitk::Image::Pointer imageVolume = reader->GetOutput();
	MITK_TEST_CONDITION_REQUIRED(imageVolume.IsNotNull(), "Image required");


	mitk::PlaneGeometry::Pointer geometryTransversal = mitk::PlaneGeometry::New();

	geometryTransversal->InitializeStandardPlane(imageVolume->GetGeometry(), mitk::PlaneGeometry::PlaneOrientation::Transversal, 0, false, true);
	geometryTransversal->SetImageGeometry(true);

	//bool testTransversal = TestSlice(imageVolume, geometryTransversal, "C:\\home\\schroedt\\MITK\\Modules\\ImageExtraction\\Testing\\Data\\img.nrrd");//pic3d-transversal-(0.0.0).pic");
	//MITK_TEST_CONDITION( testTransversal, "Testing slice extraction");




/*************** #BEGIN vtk render code ************************/
	
//set reslicer for renderwindow
	mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
	slicer->SetInput(imageVolume);
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
	lookupTable->SetRange(-1022.0, 1184.0);

	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();

	MITK_INFO << "spacing: " << slicer->GetOutput()->GetGeometry()->GetSpacing() << " |> origin: " << slicer->GetOutput()->GetGeometry()->GetOrigin() << "|> isImageGeometry: " << slicer->GetOutput()->GetGeometry()->GetImageGeometry();

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
	MITK_TEST_END()
}