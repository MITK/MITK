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
#include "mitkDataNodeFactory.h"
#include "mitkTestingMacros.h"
#include <iostream>
#include <fstream>

#ifdef WIN32
#include "process.h"
#endif

std::string AppendExtension(const std::string &filename, const char *extension)
{
  std::string new_filename = filename;

  new_filename += extension;
  return new_filename;
}

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
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();

  try
  {
    MITK_TEST_OUTPUT(<< "Loading file: " << argv[1]);
    factory->SetFileName( argv[1] );
    factory->Update();
    MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfOutputs() > 0, "file loaded");
    
    mitk::DataNode::Pointer node = factory->GetOutput( 0 );
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

  std::stringstream filename_stream;

#ifdef WIN32
  filename_stream << "test" << _getpid();
#else
  filename_stream << "test" << getpid();
#endif
  

  std::string filename = filename_stream.str();

  // test set/get methods
  myImageWriter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED(myImageWriter->GetInput()==image,"test Set/GetInput()");
  myImageWriter->SetFileName(filename);
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFileName(),filename.c_str()),"test Set/GetFileName()");
  myImageWriter->SetFilePrefix("pref");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFilePrefix(),"pref"),"test Set/GetFilePrefix()");
  myImageWriter->SetFilePattern("pattern");
  MITK_TEST_CONDITION_REQUIRED(!strcmp(myImageWriter->GetFilePattern(),"pattern"),"test Set/GetFilePattern()");

  // write ITK .mhd image (2D and 3D only)
  if( image->GetDimension() <= 3 )
  {
    try
    {
      myImageWriter->SetExtension(".mhd");
      myImageWriter->Update();
      std::fstream fin, fin2;
      fin.open(AppendExtension(filename, ".mhd").c_str(),std::ios::in);
      fin2.open(AppendExtension(filename, ".raw").c_str(),std::ios::in);
      MITK_TEST_CONDITION_REQUIRED(fin.is_open(),"Write .mhd file");
      MITK_TEST_CONDITION_REQUIRED(fin2.is_open(),"Write .raw file");
      fin.close();
      fin2.close();
      remove(AppendExtension(filename, ".mhd").c_str());
      remove(AppendExtension(filename, ".raw").c_str());
    }
    catch (...)
    {
      MITK_TEST_FAILED_MSG(<< "Exception during .mhd file writing");
    }
  }

  //testing more component image writing as nrrd files
  try
  {
    myImageWriter->SetExtension(".nrrd");
    myImageWriter->Update();
    std::fstream fin;
    fin.open(AppendExtension(filename, ".nrrd").c_str(),std::ios::in);
    MITK_TEST_CONDITION_REQUIRED(fin.is_open(),"Write .nrrd file");
    fin.close();
    remove(AppendExtension(filename, ".nrrd").c_str());
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG(<< "Exception during .nrrd file writing");
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
