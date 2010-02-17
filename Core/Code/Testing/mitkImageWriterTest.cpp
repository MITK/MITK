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
#include <fstream>

/**
*  test for "ImageWriter".
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
    
  MITK_TEST_CONDITION_REQUIRED(argc != 0, "File to load has been specified");


  mitk::Image::Pointer image = NULL;
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    MITK_TEST_OUTPUT(<< "Loading file: " << argv[1]);
    factory->SetFileName( argv[1] );
    factory->Update();
    MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file loaded");
    
    mitk::DataTreeNode::Pointer node = factory->GetOutput( 0 );
    image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
      std::cout<<"file "<< argv[1]<< "is not an image - test will not be applied."<<std::endl;
      std::cout<<"[TEST DONE]"<<std::endl;
      return EXIT_SUCCESS;
    }  
  }
  catch (itk::ExceptionObject & ex)
  {
    MITK_TEST_FAILED_MSG(<< "Exception during file loading: " << ex.GetDescription());
  }


  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(),"loaded image not NULL")

  // test set/get methods
  myImageWriter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED(myImageWriter->GetInput()==image,"test Set/GetInput()");
  myImageWriter->SetFileName("test");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFileName(),"test"),"test Set/GetFileName()");
  myImageWriter->SetExtension(".pic");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetExtension(),".pic"),"test Set/GetExtension()");
  myImageWriter->SetFilePrefix("pref");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFilePrefix(),"pref"),"test Set/GetFilePrefix()");
  myImageWriter->SetFilePattern("pattern");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFilePattern(),"pattern"),"test Set/GetFilePattern()");
  // write image
  try
  {
    myImageWriter->Update();
    bool flag = false;
    std::fstream fin;
    fin.open("test.pic",std::ios::in);
    MITK_TEST_CONDITION_REQUIRED(fin.is_open(),"Write file");
    fin.close();
    remove("test.pic");
  }
  catch (...)
  {
    MITK_TEST_FAILED_MSG(<< "Exception during file writing");
  }

  // test for exception handling
  try
  {
    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
    myImageWriter->SetInput(image);
    myImageWriter->SetFileName("/usr/bin");
    myImageWriter->Update(); 
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown 
    MITK_TEST_FAILED_MSG(<< "Wrong exception (i.e. no itk:Exception) caught during write");
  }

  // always end with this!
  MITK_TEST_END();
}
