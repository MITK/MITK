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
#include "mitkException.h"

#include <mitkExtractSliceFilter.h>
#include "mitkIOUtil.h"

#include "itksys/SystemTools.hxx"

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

bool CompareImageMetaData( mitk::Image::Pointer image, mitk::Image::Pointer reference, bool checkPixelType = true )
{
  // switch to AreIdentical() methods as soon as Bug 11925 (Basic comparison operators) is fixed

  if( image->GetDimension() != reference->GetDimension() )
  {
    MITK_ERROR << "The image dimension differs: IN (" << image->GetDimension() << ") REF(" << reference->GetDimension() << ")";
    return false;
  }

  // pixel type
  if( checkPixelType &&
      ( image->GetPixelType() != reference->GetPixelType()
      && image->GetPixelType().GetBitsPerComponent() != reference->GetPixelType().GetBitsPerComponent() ) )
  {
    MITK_ERROR << "Pixeltype differs ( image=" << image->GetPixelType().GetPixelTypeAsString() << "[" << image->GetPixelType().GetBitsPerComponent() << "]" << " reference=" << reference->GetPixelType().GetPixelTypeAsString() << "[" << reference->GetPixelType().GetBitsPerComponent() << "]" << " )";
    return false;
  }

  return true;
}


/*
Test writing picture formats like *.bmp, *.png, *.tiff or *.jpg
NOTE: Saving as picture format must ignore PixelType comparison - not all bits per components are supported (see specification of the format)
*/
void TestPictureWriting(mitk::Image* image, const std::string& filename, const std::string& extension)
{
  const std::string fullFileName = AppendExtension(filename, extension.c_str());

  mitk::Image::Pointer singleSliceImage = NULL;
  if( image->GetDimension() == 3 )
  {
    mitk::ExtractSliceFilter::Pointer extractFilter = mitk::ExtractSliceFilter::New();
    extractFilter->SetInput( image );
    extractFilter->SetWorldGeometry( image->GetSlicedGeometry()->GetPlaneGeometry(0) );

    extractFilter->Update();
    singleSliceImage = extractFilter->GetOutput();

    // test 3D writing in format supporting only 2D
    mitk::IOUtil::Save(image, fullFileName);

    // test images
    unsigned int foundImagesCount = 0;

    //if the image only contains one sinlge slice the itkImageSeriesWriter won't add a number like filename.XX.extension
    if(image->GetDimension(2) == 1)
    {
      std::stringstream series_filenames;
      series_filenames << filename << extension;
      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage( series_filenames.str() );
      if( compareImage.IsNotNull() )
      {
        foundImagesCount++;
        MITK_TEST_CONDITION(CompareImageMetaData( singleSliceImage, compareImage, false ), "Image meta data unchanged after writing and loading again. "); //ignore bits per component
      }
      remove( series_filenames.str().c_str() );
    }
    else //test the whole slice stack
    {
      for( unsigned int i=0; i< image->GetDimension(2); i++)
      {
        std::stringstream series_filenames;
        series_filenames << filename << "." << i+1 << extension;
        mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage( series_filenames.str() );
        if( compareImage.IsNotNull() )
        {
          foundImagesCount++;
          MITK_TEST_CONDITION(CompareImageMetaData( singleSliceImage, compareImage, false ), "Image meta data unchanged after writing and loading again. "); //ignore bits per component
        }
        remove( series_filenames.str().c_str() );
      }
    }
    MITK_TEST_CONDITION( foundImagesCount == image->GetDimension(2), "All 2D-Slices of a 3D image were stored correctly.");
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
      mitk::IOUtil::Save(singleSliceImage, fullFileName);

      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(fullFileName.c_str());
      MITK_TEST_CONDITION_REQUIRED( compareImage.IsNotNull(), "Image stored was succesfully loaded again");

      MITK_TEST_CONDITION_REQUIRED( CompareImageMetaData(singleSliceImage, compareImage, false ), "Image meta data unchanged after writing and loading again. ");//ignore bits per component
      remove(fullFileName.c_str());
    }
    catch(itk::ExceptionObject &e)
    {
      MITK_TEST_FAILED_MSG(<< "Exception during file writing for ." << extension << ": " << e.what() );
    }

  }

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

    // write ITK .mhd image (2D and 3D only)
  if( image->GetDimension() <= 3 )
  {
    std::ofstream tmpStream;
    std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.mhd");
    tmpStream.close();

    std::string tmpFilePathWithoutExt = tmpFilePath.substr(0, tmpFilePath.size() - 4);

    try
    {
      mitk::IOUtil::Save(image, tmpFilePath);

      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(tmpFilePath);
      MITK_TEST_CONDITION_REQUIRED( compareImage.IsNotNull(), "Image stored in MHD format was succesfully loaded again! ");

      MITK_TEST_CONDITION(itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".mhd").c_str()), ".mhd file exists")
      MITK_TEST_CONDITION(itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".raw").c_str()) ||
                          itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".zraw").c_str()), ".raw or .zraw exists")

      // delete
      remove(tmpFilePath.c_str());
      remove((tmpFilePathWithoutExt + ".raw").c_str());
      remove((tmpFilePathWithoutExt + ".zraw").c_str());
    }
    catch (const std::exception& e)
    {
      MITK_TEST_FAILED_MSG(<< "Exception during .mhd file writing: " << e.what());
    }
  }

  //testing more component image writing as nrrd files

  {
    std::ofstream tmpStream;
    std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.nrrd");
    tmpStream.close();

    try
    {
      mitk::IOUtil::Save(image, tmpFilePath);

      mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(tmpFilePath);
      MITK_TEST_CONDITION_REQUIRED(compareImage.IsNotNull(), "Image stored in NRRD format was succesfully loaded again");

      remove(tmpFilePath.c_str());
    }
    catch(...)
    {
      std::remove(tmpFilePath.c_str());
      MITK_TEST_FAILED_MSG(<< "Exception during .nrrd file writing");
    }
  }

  std::ofstream tmpStream;
  std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX");
  tmpStream.close();

  TestPictureWriting(image, tmpFilePath, ".png");
  TestPictureWriting(image, tmpFilePath, ".jpg");
  TestPictureWriting(image, tmpFilePath, ".tiff");
  TestPictureWriting(image, tmpFilePath, ".bmp");

  // test for exception handling
  try
  {
    mitk::IOUtil::Save(image, "/usr/bin");
    MITK_TEST_FAILED_MSG( << "mitk::Exception expected")
  }
  catch (const mitk::Exception&) { /* this is expected */ }
  catch (...)
  {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown
    MITK_TEST_FAILED_MSG(<< "Wrong exception (i.e. no itk:Exception) caught during write");
  }

  // always end with this!
  MITK_TEST_END();
}
