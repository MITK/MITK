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
//
//#include "mitkExtractDirectedPlaneImageFilter.h"
//#include "mitkStandardFileLocations.h"
//
//#include <vtkImageData.h>
//#include <vtkSmartPointer.h>
//#include <mitkItkImageFileReader.h>
//#include <vtkActor.h>
//#include <vtkRenderWindowInteractor.h>
//
//#include "mitkTestingMacros.h"
//
//#include <iostream>
//
//
//class ExtractionTesting{
//
//public:
//
//  struct Testcase
//  {
//    int number;
//    std::string name;
//    std::string imageFilename;
//    std::string referenceImageFilename;
//    bool success;
//    mitk::Geometry2D::Pointer (*GetPlane) (void);
//  };
//
//  static void DoTesting(Testcase &testcase)
//  {
//    mitk::Image::Pointer image = GetImageToTest(testcase.imageFilename);
//    if ( image.IsNull){
//      testcase.success = false;
//      return;
//    }
//
//    /*mitk::Image::Pointer referenceImage = GetImageToTest(testcase.referenceImageFilename);
//    if ( referenceImage.IsNull){
//      testcase.success = false;
//      return;
//    }
//
//    mitk::Geometry2D::Pointer directedGeometry2D = testcase.GetPlane();
//    if(directedGeometry2D.IsNull){
//      testcase.success = false;*/
//    }
//
//    //put testing here
//    //TODO vtkIMageREslice setup
//    //vtkSmartPointer<vtk
//
//      vtkSmartPointer<vtkImageData> colorImage = image->GetVtkImageData();
//
//    vtkSmartPointer<vtkImageMapper3D> imageMapper = vtkSmartPointer<vtkImageMapper3D>::New();
//    imageMapper->SetInput(colorImage);
//
//
//    vtkSmartPointer<vtkActor> imageActor = vtkSmartPointer<vtkActor>::New();
//    imageActor->SetMapper(imageMapper);
//    //imageActor->SetPosition(20, 20);
//
//    // Setup renderers
//    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
//
//    // Setup render window
//    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//    renderWindow->AddRenderer(renderer);
//
//    // Setup render window interactor
//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
//    renderWindowInteractor->SetInteractorStyle(style);
//
//    // Render and start interaction
//    renderWindowInteractor->SetRenderWindow(renderWindow);
//    //renderer->AddViewProp(imageActor);
//    renderer->AddActor(imageActor);
//
//    renderWindow->Render();
//    renderWindowInteractor->Start();
//  }
//
//
//  static std::vector<testCase> InitializeTestCases()
//  {
//    int testcounter = 0;
//    std::vector<testCase> tests=
//
//    //#BEGIN setup TestCases
//
//    {
//      {
//        ++testcounter,
//        "TestCoronal",
//        "image.nrrd",
//        "coronalReference.nrrd",
//        false,
//        &TestCoronal
//      },
//      {
//        ++testcounter,
//        "TestSagital",
//        "image.nrrd",
//        "sagitalReference.nrrd",
//        false,
//        &TestSagital
//      },
//      {
//        ++testcounter,
//        "TestCoronal",
//        "image.nrrd",
//        "coronalReference.nrrd",
//        false,
//        &TestCoronal
//      },
//      {
//        ++testcounter,
//        "Test_u_Rotation",
//        "image.nrrd",
//        "uRotationReference.nrrd",
//        false,
//        &Test_u_Rotation
//      },
//      {
//        ++testcounter,
//        "Test_v_Rotation",
//        "image.nrrd",
//        "vRotationReference.nrrd",
//        false,
//        &Test_v_Rotation
//      },
//      {
//        ++testcounter,
//        "TestTwoDirectionalRation",
//        "image.nrrd",
//        "twoDirectionalRationReference.nrrd",
//        false,
//        &TestTwoDirectionalRotation
//      },
//      {
//        ++testcounter,
//        "Test4D",
//        "image.nrrd",
//        "twoDirectionalRationReference.nrrd",
//        false,
//        &Test4D
//      },
//      {
//        ++testcounter,
//        "Test2D",
//        "coronalReference.nrrd",
//        "coronalReference.nrrd",
//        false,
//        &Test2D
//      },
//      {
//        ++testcounter,
//        "Test2D",
//        NULL,
//        NULL,
//        false,
//        &Test1D
//      }
//
//    };
//
//    //#END setup TestCases
//
//    return tests;
//  }
//
//protected:
//
//  static mitk::Image::Pointer GetImageToTest(std::string filename){
//    //retrieve referenceImage
//
////  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();
////
////  const std::string filepath = locator->FindFile(filename, "Modules/MitkExt/Testing/Data");
////
////  if (filepath.empty())
////  {
////  return NULL;
////  }
////
//////TODO read imge from file
////  itk::FilenamesContainer file;
////  file.push_back( filename );
//    mitk::ItkImageFileReader::Pointer reader = mitk::ItkImageFileReader::New();
//    reader->SetFileName("C:\home\Pics\Pic3D.nrrd");
//
//    reader->Update();
//
//    mitk::Image::Pointer image = reader->GetOutput();
//
//    return image;
//  }
//
//
//  static mitk::Geometry2D::Pointer TestSagital()
//  {
//
//    return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer TestCoronal()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer TestAxial()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer Test_u_Rotation()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer Test_v_Rotation()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer TestTwoDirectionalRotation()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer Test4DImage()
//  {return NULL;
//
//  }
//
//  static mitk::Geometry2D::Pointer Test2DImage()
//  {
//return NULL;
//  }
//
//  static mitk::Geometry2D::Pointer Test1DImage()
//  {
//return NULL;
//  }
//
//};
//
//
//**
// *  Tests for the class "mitkExtractDirectedPlaneImageFilter".
// *
// *  argc and argv are the command line parameters which were passed to
// *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
// *  tests, argv is either empty for the simple tests or contains the filename
// *  of a test image for the image tests (see CMakeLists.txt).
// */
//int mitkExtractDirectedPlaneImageFilterTest(int /* argc */, char* /*argv*/[])
//{
//  // always start with this!
//  MITK_TEST_BEGIN("mitkExtractDirectedPlaneImageFilter")
//
//
//    mitk::ExtractDirectedPlaneImageFilter::Pointer extractor = mitk::ExtractDirectedPlaneImageFilter::New();
//  MITK_TEST_CONDITION_REQUIRED(extractor.IsNotNull(),"Testing instantiation")
//
//
//    std::vector<ExtractionTesting::Testcase> allTests = ExtractionTesting::InitializeTestCases();
//
//  for( int i = 0; i < allTests.size(); i++);{
//
//    ExtractionTesting::Testcase testCase = allTest[i];
//
//    ExtractionTesting::DoTesting(testCase);
//
//    MITK_TEST_CONDITION(testCase.success, "Testcase #'" << testCase.number << " " << testCase.name);
//  }
//
//  // always end with this!
//  MITK_TEST_END()
//}
