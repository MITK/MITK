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

#include <mitkTestFixture.h>
#include "mitkException.h"
#include <mitkTestingMacros.h>
#include <mitkStandardFileLocations.h>

#include <mitkExtractSliceFilter.h>
#include "mitkIOUtil.h"
#include "mitkITKImageImport.h"

#include "itksys/SystemTools.hxx"
#include <itkImageRegionIterator.h>

#include <iostream>
#include <fstream>

#ifdef WIN32
#include "process.h"
#else
#include <unistd.h>
#endif

class mitkItkImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkItkImageIOTestSuite);
  MITK_TEST(TestImageWriterJpg);
  MITK_TEST(TestImageWriterPng1);
  MITK_TEST(TestImageWriterPng2);
  MITK_TEST(TestImageWriterPng3);
  MITK_TEST(TestImageWriterSimple);
  MITK_TEST(TestWrite3DImageWithOnePlane);
  MITK_TEST(TestWrite3DImageWithTwoPlanes);
  MITK_TEST(TestWrite3DplusT_ArbitraryTG);
  MITK_TEST(TestWrite3DplusT_ProportionalTG);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {

  }

  void tearDown() override
  {
  }


  void TestImageWriterJpg()
  {
    TestImageWriter("NrrdWritingTestImage.jpg");
  }

  void TestImageWriterPng1()
  {
    TestImageWriter("Png2D-bw.png");
  }

  void TestImageWriterPng2()
  {
    TestImageWriter("RenderingTestData/rgbImage.png");
  }

  void TestImageWriterPng3()
  {
    TestImageWriter("RenderingTestData/rgbaImage.png");
  }

  void TestWrite3DplusT_ArbitraryTG()
  {
    TestImageWriter("3D+t-ITKIO-TestData/LinearModel_4D_arbitrary_time_geometry.nrrd");
  }

  void TestWrite3DplusT_ProportionalTG()
  {
    TestImageWriter("3D+t-ITKIO-TestData/LinearModel_4D_prop_time_geometry.nrrd");
  }

  void TestImageWriterSimple()
  {
    // TODO
  }

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
  *  test for writing NRRDs
  */
  void TestNRRDWriting(const mitk::Image* image)
  {
    CPPUNIT_ASSERT_MESSAGE("Internal error. Passed reference image is null.", image);

      std::ofstream tmpStream;
      std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.nrrd");
      tmpStream.close();

      try
      {
        mitk::IOUtil::Save(image, tmpFilePath);

        mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(tmpFilePath);
        CPPUNIT_ASSERT_MESSAGE("Image stored in NRRD format was succesfully loaded again", compareImage.IsNotNull());

        /*It would make sence to check the images as well (see commented cppunit assert),
          but currently there seems to be a problem (exception) with most of the test images
          (partly it seems to be a problem when try to access the pixel content by AccessByItk_1
          in mitk::CompareImageDataFilter.
          This problem should be dealt with in Bug 19533 - mitkITKImageIOTest needs improvement */
        //CPPUNIT_ASSERT_MESSAGE("Images are equal.", mitk::Equal(*image, *compareImage, mitk::eps, true));
        CPPUNIT_ASSERT_MESSAGE("TimeGeometries are equal.", mitk::Equal(*(image->GetTimeGeometry()), *(compareImage->GetTimeGeometry()), mitk::eps, true));

        remove(tmpFilePath.c_str());
      }
      catch (...)
      {
        std::remove(tmpFilePath.c_str());
        CPPUNIT_FAIL("Exception during NRRD file writing");
      }
  }

  /**
  *  test for writing MHDs
  */
  void TestMHDWriting(const mitk::Image* image)
  {
    CPPUNIT_ASSERT_MESSAGE("Internal error. Passed reference image is null.", image);

    std::ofstream tmpStream;
      std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.mhd");
      tmpStream.close();

      std::string tmpFilePathWithoutExt = tmpFilePath.substr(0, tmpFilePath.size() - 4);

      try
      {
        mitk::IOUtil::Save(image, tmpFilePath);

        mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(tmpFilePath);
        CPPUNIT_ASSERT_MESSAGE("Image stored in MHD format was succesfully loaded again! ", compareImage.IsNotNull());


        CPPUNIT_ASSERT_MESSAGE(".mhd file exists", itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".mhd").c_str()));
        CPPUNIT_ASSERT_MESSAGE(".raw or .zraw exists", itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".raw").c_str()) ||
          itksys::SystemTools::FileExists((tmpFilePathWithoutExt + ".zraw").c_str()));

        /*It would make sence to check the images as well (see commented cppunit assert),
        but currently there seems to be a problem (exception) with most of the test images
        (partly it seems to be a problem when try to access the pixel content by AccessByItk_1
        in mitk::CompareImageDataFilter.
        This problem should be dealt with in Bug 19533 - mitkITKImageIOTest needs improvement */
        //CPPUNIT_ASSERT_MESSAGE("Images are equal.", mitk::Equal(*image, *compareImage, mitk::eps, true));
        CPPUNIT_ASSERT_MESSAGE("TimeGeometries are equal.", mitk::Equal(*(image->GetTimeGeometry()), *(compareImage->GetTimeGeometry()), 5e-4, true));

        // delete
        remove(tmpFilePath.c_str());
        remove((tmpFilePathWithoutExt + ".raw").c_str());
        remove((tmpFilePathWithoutExt + ".zraw").c_str());
      }
      catch (...)
      {
        CPPUNIT_FAIL("Exception during.mhd file writing");
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
  void TestImageWriter(std::string sourcefile)
  {
    sourcefile = GetTestDataFilePath(sourcefile);

    // load image
    CPPUNIT_ASSERT_MESSAGE("Checking whether source image exists", itksys::SystemTools::FileExists(sourcefile.c_str()));

    mitk::Image::Pointer image = NULL;

    try
    {
      image = mitk::IOUtil::LoadImage( sourcefile );
    }
    catch (...)
    {
      CPPUNIT_FAIL("Exception during file loading:");
    }

    CPPUNIT_ASSERT_MESSAGE("loaded image not NULL", image.IsNotNull());

    // write ITK .mhd image (2D and 3D only)
    if( image->GetDimension() <= 3 )
    {
      TestMHDWriting(image);
    }

    //testing more component image writing as nrrd files
    TestNRRDWriting(image);

    std::ofstream tmpStream;
    std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX");
    tmpStream.close();

    TestPictureWriting(image, tmpFilePath, ".png");
    TestPictureWriting(image, tmpFilePath, ".jpg");
    TestPictureWriting(image, tmpFilePath, ".tiff");
    TestPictureWriting(image, tmpFilePath, ".bmp");
    // always end with this!

  }

  /**
  * Try to write a 3D image with only one plane (a 2D images in disguise for all intents and purposes)
  */
  void TestWrite3DImageWithOnePlane(){
    typedef itk::Image<unsigned char, 3>  ImageType;

    ImageType::Pointer itkImage = ImageType::New();

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 1;

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
    itkImage->SetRegions(region);
    itkImage->Allocate();
    itkImage->FillBuffer(0);

    itk::ImageRegionIterator<ImageType> imageIterator(itkImage, itkImage->GetLargestPossibleRegion());

    // Make two squares
    while (!imageIterator.IsAtEnd())
    {
      if ((imageIterator.GetIndex()[0] > 5 && imageIterator.GetIndex()[0] < 20) &&
        (imageIterator.GetIndex()[1] > 5 && imageIterator.GetIndex()[1] < 20))
      {
        imageIterator.Set(255);
      }

      if ((imageIterator.GetIndex()[0] > 50 && imageIterator.GetIndex()[0] < 70) &&
        (imageIterator.GetIndex()[1] > 50 && imageIterator.GetIndex()[1] < 70))
      {
        imageIterator.Set(60);
      }
      ++imageIterator;
    }

    mitk::Image::Pointer image = mitk::ImportItkImage(itkImage);

    mitk::IOUtil::SaveImage(image, mitk::IOUtil::CreateTemporaryFile("3Dto2DTestImageXXXXXX.nrrd"));
    mitk::IOUtil::SaveImage(image, mitk::IOUtil::CreateTemporaryFile("3Dto2DTestImageXXXXXX.png"));

  }

  /**
  * Try to write a 3D image with only one plane (a 2D images in disguise for all intents and purposes)
  */
  void TestWrite3DImageWithTwoPlanes(){
    typedef itk::Image<unsigned char, 3>  ImageType;

    ImageType::Pointer itkImage = ImageType::New();

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 2;

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
    itkImage->SetRegions(region);
    itkImage->Allocate();
    itkImage->FillBuffer(0);

    itk::ImageRegionIterator<ImageType> imageIterator(itkImage, itkImage->GetLargestPossibleRegion());

    // Make two squares
    while (!imageIterator.IsAtEnd())
    {
      if ((imageIterator.GetIndex()[0] > 5 && imageIterator.GetIndex()[0] < 20) &&
        (imageIterator.GetIndex()[1] > 5 && imageIterator.GetIndex()[1] < 20))
      {
        imageIterator.Set(255);
      }
      if ((imageIterator.GetIndex()[0] > 50 && imageIterator.GetIndex()[0] < 70) &&
        (imageIterator.GetIndex()[1] > 50 && imageIterator.GetIndex()[1] < 70))
      {
        imageIterator.Set(60);
      }
      ++imageIterator;
    }
    mitk::Image::Pointer image = mitk::ImportItkImage(itkImage);

    mitk::IOUtil::SaveImage(image, mitk::IOUtil::CreateTemporaryFile("3Dto2DTestImageXXXXXX.nrrd"));

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::SaveImage(image, mitk::IOUtil::CreateTemporaryFile("3Dto2DTestImageXXXXXX.png")), mitk::Exception);

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkItkImageIO)
