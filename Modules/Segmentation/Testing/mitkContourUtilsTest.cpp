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
#include "mitkTestingMacros.h"
#include "mitkContourUtils.h"
#include "mitkContourModel.h"

//file loading
#include <fstream>
#include "mitkDataNodeFactory.h"

#include "mitkContourModelReader.h"

  void TestContourModel2itkMesh()
  {
    mitk::ContourModel::Pointer contourModel = mitk::ContourModel::New();
    mitk::ContourUtils::Pointer utils = mitk::ContourUtils::New();

    mitk::Point3D p,p1,p2,p3;

    //load a image file for testing
    std::string path = "/home/woitzik/mitk/src/MITK/Modules/Segmentation/Testing/Data/Sigmoid.nrrd";

    mitk::Image::Pointer image = NULL;
    mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
    try
    {
      std::cout << "Loading file: "<<std::flush;
      factory->SetFileName( path.c_str() );
      factory->Update();

      if(factory->GetNumberOfOutputs()<1)
      {
        std::cout<<"file could not be loaded [FAILED]"<<std::endl;
        return;
      }
      mitk::DataNode::Pointer node = factory->GetOutput( 0 );
      image = dynamic_cast<mitk::Image*>(node->GetData());
      if(image.IsNull())
      {
        std::cout<<"file not an image - test will not be applied [PASSED]"<<std::endl;
        std::cout<<"[TEST DONE]"<<std::endl;
        return;
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      std::cout << "Exception: " << ex << "[FAILED]" << std::endl;
      return;
    }


    p[0] = 0;
    p[1] = 0;
    p[2] = 0;

    p1[0] = 100;
    p1[1] = 0;
    p1[2] = 0;

    p2[0] = 100;
    p2[1] = 100;
    p2[2] = 0;

    p3[0] = 0;
    p3[1] = 100;
    p3[2] = 0;

    contourModel->AddVertex(p);
    contourModel->AddVertex(p1);
    contourModel->AddVertex(p2);
    contourModel->AddVertex(p3);

    //mitk::ContourModelReader::Pointer contourReader = mitk::ContourModelReader::New();
    //contourReader->SetFileName("/home/woitzik/TestingMethodeOutput/workingContourNode.cnt");
    //contourReader->Update();
    //contour = contourReader->GetOutput();

    //test FillContourInSlice
    utils->FillContourInSlice( contourModel, 0, image, 1 );

    //MITK_INFO << utils->m_mesh;
    MITK_INFO << "should be done";

  }

  int mitkContourUtilsTest(int argc, char* argv[])
  {
    MITK_TEST_BEGIN("mitkContourUtilsTest")

    TestContourModel2itkMesh();

    MITK_TEST_END()
  }

