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

#include "mitkPicFileWriter.h"
#include "mitkDataNodeFactory.h"
#include "mitkTestingMacros.h"

#include "mitkTestingConfig.h"
/**
*  Simple example for a test for the (non-existent) class "PicFileWriter".
*  
*  argc and argv are the command line parameters which were passed to 
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/
int mitkPicFileWriterTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("PicFileWriter")

    // let's create an object of our class  
    mitk::PicFileWriter::Pointer myPicFileWriter = mitk::PicFileWriter::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myPicFileWriter.IsNotNull(),"Testing instantiation") 

    // load image
    std::cout << "Loading file: " << std::flush;
  if(argc==0)
  {
    std::cout<<"no file specified [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  mitk::Image::Pointer image = NULL;

  try{  
    // test for exception handling of NULL image
    std::cout << "Testing handling of NULL image " << std::flush;
    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)

    myPicFileWriter->SetInputImage(image);
    myPicFileWriter->SetFileName("/usr/bin");
    myPicFileWriter->Update(); 
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    std::cout << "Success: Writer warns on NULL image." << std::endl;
  }

  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
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
    mitk::DataNode::Pointer node = factory->GetOutput( 0 );
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

    myPicFileWriter->SetInputImage(image);
    myPicFileWriter->SetFileName("/usr/bin");
    myPicFileWriter->Update(); 
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  unsigned char data_channel1[8] = {0,1,2,3,4,5,6,7};
  unsigned char data_channel2[8] = {10,11,12,13,14,15,16,17};

  unsigned int dims[2] = {4,2};

  // create a two-channel test image
  mitk::Image::Pointer mcImage = mitk::Image::New();
  mcImage->Initialize( mitk::MakeScalarPixelType<unsigned char>(), 2, dims, 2);
  mcImage->SetChannel( data_channel1, 0);
  mcImage->SetChannel( data_channel2, 1);


  // create a file name that can be saved so that the writer does not throw exception because of unwriteable location
  std::string filename = std::string( MITK_TEST_OUTPUT_DIR ) + "MultiChannel.pic";

  MITK_TEST_CONDITION_REQUIRED(mcImage.IsNotNull(),"The multi-channel image for testing is not NULL")

  try{
    // test for exception handling
    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)

    mitk::PicFileWriter::Pointer myPicFileWriter2 = mitk::PicFileWriter::New();

    myPicFileWriter2->SetInputImage(mcImage);
    myPicFileWriter2->SetFileName(filename);
    myPicFileWriter2->Update();
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

