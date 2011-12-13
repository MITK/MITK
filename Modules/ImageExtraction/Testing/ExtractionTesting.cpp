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

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <mitkItkImageFileReader.h>
#include <vtkActor.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapper3D.h>

#include "mitkTestingMacros.h"

int mitkExtractionTestingTest(int /* argc */, char* /*argv*/[])
{
	// always start with this!
	MITK_TEST_BEGIN("mitkExtractDirectedPlaneImageFilter")

		mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
	reader->SetFileName("C:\home\Pics\Pic3D.nrrd");

	reader->Update();

	mitk::Image::Pointer image = reader->GetOutput();

	//TODO vtkIMageREslice setup
	//vtkSmartPointer<vtk

	vtkSmartPointer<vtkImageData> colorImage = image->GetVtkImageData();

	vtkSmartPointer<vtkImageMapper3D> imageMapper = vtkSmartPointer<vtkImageMapper3D>::New();
	imageMapper->SetInput(colorImage);


	vtkSmartPointer<vtkActor> imageActor = vtkSmartPointer<vtkActor>::New();
	imageActor->SetMapper(imageMapper);
	//imageActor->SetPosition(20, 20);

	// Setup renderers
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

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
	renderer->AddActor(imageActor);

	renderWindow->Render();
	renderWindowInteractor->Start();
	// always end with this!
	MITK_TEST_END()
}