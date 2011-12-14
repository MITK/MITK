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


int mitkExtractSliceFilterTest(int argc, char* argv[])
{
	// always start with this!
	MITK_TEST_BEGIN("mitkExtractSliceFilterTest")


	mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();

	std::string filename = "C:\\home\\Pics\\Pic3D.nrrd";//"C:\\home\\schroedt\\bin_mitk\\CMakeExternals\\Source\\MITK-Data\\NrrdWritingTestImage.jpg";
	reader->SetFileName(filename);
	
	reader->Update();

	mitk::Image::Pointer image = reader->GetOutput();


	mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();

	mitk::PlaneGeometry::Pointer geometry = mitk::PlaneGeometry::New();

	mitk::Vector3D right, bottom, normal;
	mitk::Point3D origin;

	mitk::FillVector3D(origin, 0.0, 0.0, 0.0);

	/*mitk::FillVector3D(right, 1.0, 0.0, 0.0);
	mitk::FillVector3D(bottom, 0.0, -1.0, 0.0);

	normal = itk::CrossProduct(right, bottom);*/

	geometry->InitializeStandardPlane(image->GetGeometry(), mitk::PlaneGeometry::PlaneOrientation::Transversal, 24, false, true);
	geometry->SetOrigin(origin);
	
	/*mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 1, 0, 0 );
	mitk::RotationOperation* rotation = new mitk::RotationOperation( mitk::OpROTATE, geometry->GetCenter(), rotationVector, 180.0);
	geometry->ExecuteOperation(rotation);
	delete rotation;*/

	MITK_INFO << "axis 0: " << geometry->GetAxisVector(0) << " |> aixs 1: " << geometry->GetAxisVector(1) << " |> center: " << geometry->GetCenter();

	slicer->SetInput(image);
	slicer->SetWorldGeometry(geometry);

	slicer->Update();

	MITK_TEST_CONDITION(slicer->GetOutput() != NULL, " Testing resliced Image returned");


/*************** #BEGIN vtk render code ************************/
	vtkSmartPointer<vtkPlaneSource> m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
	m_Plane->SetOrigin(0.0, 0.0, 0.0);
	//These two points define the axes of the plane in combination with the origin.
	//Point 1 is the x-axis and point 2 the y-axis.
	//Each plane is transformed according to the view (transversal, coronal and saggital) afterwards.
	m_Plane->SetPoint1(1.0, 0.0, 0.0); //P1: (xMax, yMin, depth)
	m_Plane->SetPoint2(0.0, 1.0, 0.0); //P2: (xMin, yMax, depth)


	vtkSmartPointer<vtkPolyDataMapper> imageMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	imageMapper->SetInputConnection(m_Plane->GetOutputPort());

	vtkSmartPointer<vtkLookupTable> m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();

	//built a default lookuptable
	m_LookupTable->SetRampToLinear();
	m_LookupTable->SetSaturationRange( 0.0, 0.0 );
	m_LookupTable->SetHueRange( 0.0, 0.0 );
	m_LookupTable->SetValueRange( 0.0, 1.0 );
	m_LookupTable->Build();
	//map all black values to transparent
	m_LookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	m_LookupTable->SetRange(-1022.0, 1184.0);

	vtkSmartPointer<vtkTexture> m_Texture = vtkSmartPointer<vtkTexture>::New();

	m_Texture->SetInput(slicer->GetOutput()->GetVtkImageData());
	//m_Texture->SetInput(image->GetVtkImageData());
	m_Texture->SetLookupTable(m_LookupTable);

	m_Texture->SetMapColorScalarsThroughLookupTable(true);

	vtkSmartPointer<vtkActor> imageActor = vtkSmartPointer<vtkActor>::New();
	imageActor->SetMapper(imageMapper);
	imageActor->SetTexture(m_Texture);
	

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