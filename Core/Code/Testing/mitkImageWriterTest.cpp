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

#include "mitkImageWriter.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkTestingMacros.h"

#include <iostream>

/**
*  Simple example for a test for the (non-existent) class "ImageWriter".
*  
*  argc and argv are the command line parameters which were passed to 
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/
int mitkImageWriterTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("ImageWriter")

    // let's create an object of our class  
    mitk::ImageWriter::Pointer myImageWriter = mitk::ImageWriter::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myImageWriter.IsNotNull(),"Testing instantiation") 

    // write your own tests here and use the macros from mitkTestingMacros.h !!!
    // do not write to std::cout and do not return from this function yourself!

    // load image
    std::cout << "Loading file: " << std::flush;
  if(argc==0)
  {
    std::cout<<"no file specified [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  mitk::Image::Pointer image = NULL;
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
  try
  {
    std::cout<<argv[1]<<std::endl;
    factory->SetFileName( argv[1] );
    factory->Update();

    if(factory->GetNumberOfOutputs()<1)
    {
      std::cout<<"file could not be loaded [FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
    image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
      std::cout<<"file "<< argv[1]<< "is not an image - test will not be applied [PASSED]"<<std::endl;
      std::cout<<"[TEST DONE]"<<std::endl;
      return EXIT_SUCCESS;
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    std::cout << "Exception: " << ex << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(),"loaded image not NULL")

    try{  
      // test for exception handling
      MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      myImageWriter->SetInput(image);
      myImageWriter->SetFileName("/usr/bin");
      myImageWriter->Update(); 
      MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END()
}

