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

#include "mitkImageWriter.h"
#include "mitkDataNodeFactory.h"
#include "mitkTestingMacros.h"
#include "mitkItkImageFileReader.h"
#include "mitkException.h"

#include <mitkExtractSliceFilter.h>
#include "mitkIOUtil.h"

#include <iostream>
#include <fstream>

#ifdef WIN32
#include "process.h"
#else
#include <unistd.h>
#endif

std::string AppendExtension(const std::string &filename, const char *extension)
{
  std::string new_filename = filename;

  new_filename += extension;
  return new_filename;
}

bool CompareImageMetaData( mitk::Image::Pointer image, mitk::Image::Pointer reference)
{
  // switch to AreIdentical() methods as soon as Bug 11925 (Basic comparison operators) is fixed

  if( image->GetDimension() != reference->GetDimension() )
  {
    MITK_ERROR << "The image dimension differs: IN (" << image->GetDimension() << ") REF(" << reference->GetDimension() << ")";
    return false;
  }

  // pixel type
  if( image->GetPixelType() != reference->GetPixelType()
      && image->GetPixelType().GetBitsPerComponent() != reference->GetPixelType().GetBitsPerComponent() )
  {
    MITK_ERROR << "Pixeltype differs ";
    return false;
  }

  return true;
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
  MITK_TEST_CONDITION_REQUIRED(argc > 1, "File to load has been specified");


  mitk::Image::Pointer image = NULL;

  try
  {
    MITK_TEST_OUTPUT(<< "Loading file: " << argv[1]);
    image = mitk::IOUtil::LoadImage( argv[1] );
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
  std::cout << filename << std::endl;

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

      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage( AppendExtension(filename, ".mhd").c_str() );
      MITK_TEST_CONDITION_REQUIRED( compareImage.IsNotNull(), "Image stored in MHD format was succesfully loaded again! ");

      std::string rawExtension = ".raw";
      std::fstream rawPartIn;
      rawPartIn.open(AppendExtension(filename, ".raw").c_str());
      if( !rawPartIn.is_open() )
      {
        rawExtension = ".zraw";
        rawPartIn.open(AppendExtension(filename, ".zraw").c_str());
      }

      MITK_TEST_CONDITION_REQUIRED(rawPartIn.is_open(),"Write .raw file");
      rawPartIn.close();

      // delete
      remove(AppendExtension(filename, ".mhd").c_str());
      remove(AppendExtension(filename, rawExtension.c_str()).c_str());
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
    mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(AppendExtension(filename, ".nrrd").c_str());
    MITK_TEST_CONDITION_REQUIRED(compareImage.IsNotNull(), "Image stored in NRRD format was succesfully loaded again");
    fin.close();
    remove(AppendExtension(filename, ".nrrd").c_str());
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG(<< "Exception during .nrrd file writing");
  }


  mitk::Image::Pointer singleSliceImage = NULL;
  if( image->GetDimension() == 3 )
  {
    mitk::ExtractSliceFilter::Pointer extractFilter = mitk::ExtractSliceFilter::New();
    extractFilter->SetInput( image );
    extractFilter->SetWorldGeometry( image->GetSlicedGeometry()->GetGeometry2D(0) );

    extractFilter->Update();
    singleSliceImage = extractFilter->GetOutput();

    // test 3D writing in format supporting only 2D
    myImageWriter->SetExtension(".png");
    myImageWriter->Update();

    // test images
    unsigned int foundImagesCount = 0;
    for( unsigned int i=0; i< image->GetDimension(2); i++)
    {
      std::stringstream series_filenames;
      series_filenames << filename << "." << i+1 << ".png";
      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage( series_filenames.str() );
      if( compareImage.IsNotNull() )
      {
        foundImagesCount++;
        MITK_TEST_CONDITION(CompareImageMetaData( singleSliceImage, compareImage ), "Image meta data unchanged after writing and loading again. ");
      }
      remove( series_filenames.str().c_str() );
    }
    MITK_TEST_CONDITION( foundImagesCount == image->GetDimension(2), "All 2D-Slices of a 3D image were stored correctly as PNGs.");
  }
  else if( image->GetDimension() == 2 )
  {
    singleSliceImage = image;
  }

  // test 2D writing
  if( singleSliceImage.IsNotNull() )
  {
    try
    {
      myImageWriter->SetInput( singleSliceImage );
      myImageWriter->SetExtension(".png");
      myImageWriter->Update();

      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage( AppendExtension(filename, ".png").c_str());
      MITK_TEST_CONDITION_REQUIRED( compareImage.IsNotNull(), "Image stored in PNG format was succesfully loaded again");

      MITK_TEST_CONDITION_REQUIRED( CompareImageMetaData(singleSliceImage, compareImage ), "Image meta data unchanged after writing and loading again. ");
      remove(AppendExtension(filename, ".png").c_str());
    }
    catch(itk::ExceptionObject &e)
    {
      MITK_TEST_FAILED_MSG(<< "Exception during .png file writing: " << e.what() );
    }

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
