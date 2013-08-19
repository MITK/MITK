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

#include <mitkTestingMacros.h>
#include <mitkToFNrrdImageWriter.h>
#include <mitkImageGenerator.h>
#include <mitkImageSliceSelector.h>
#include "mitkItkImageFileReader.h"
#include "mitkImageReadAccessor.h"

/**Documentation
 *  test for the class "ToFImageWriter".
 */
int mitkToFNrrdImageWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFNrrdImageWriter");

  mitk::ToFNrrdImageWriter::Pointer tofNrrdWriter = mitk::ToFNrrdImageWriter::New();
  // testing correct initialization
  MITK_TEST_CONDITION_REQUIRED(tofNrrdWriter.GetPointer(), "Testing initialization of test object!");
  MITK_TEST_CONDITION_REQUIRED(tofNrrdWriter->GetExtension() == ".nrrd", "testing initialization of extension member variable!");

  //GENERATE TEST DATA
  ////run the test with some unusual parameters
  unsigned int dimX = 255;
  unsigned int dimY = 178;
  unsigned int pixelNumber = dimX*dimY;
  unsigned int numOfFrames = 23; //or numberOfSlices
  ////create 3 images filled with random values
  mitk::Image::Pointer distanceImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,1.0, 1.0f, 1.0f);
  mitk::Image::Pointer amplitudeImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,1.0, 1.0f, 2000.0f);
  mitk::Image::Pointer intensityImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,1.0, 1.0f, 100000.0f);

  //SET NEEDED PARAMETER
  //file names on the disc
  std::string distanceImageFileName("distImg.nrrd");
  std::string amplitudeImageFileName("amplImg.nrrd");
  std::string intensityImageFileName("intImg.nrrd");

  // set file name methods
  tofNrrdWriter->SetDistanceImageFileName(distanceImageFileName);
  tofNrrdWriter->SetAmplitudeImageFileName(amplitudeImageFileName);
  tofNrrdWriter->SetIntensityImageFileName(intensityImageFileName);
  tofNrrdWriter->SetToFCaptureWidth(dimX);
  tofNrrdWriter->SetToFCaptureHeight(dimY);
  tofNrrdWriter->SetToFImageType(mitk::ToFNrrdImageWriter::ToFImageType3D);
  tofNrrdWriter->SetRGBImageSelected(false);

  //buffer for each slice
  float* distanceArray;
  float* amplitudeArray;
  float* intensityArray;

  float* distanceArrayRead;
  float* amplitudeArrayRead;
  float* intensityArrayRead;

  tofNrrdWriter->Open(); //open file/stream

  for(unsigned int i = 0; i < numOfFrames ; i++)
  {
    mitk::ImageReadAccessor distAcc(distanceImage, distanceImage->GetSliceData(i, 0, 0));
    mitk::ImageReadAccessor amplAcc(amplitudeImage, amplitudeImage->GetSliceData(i, 0, 0));
    mitk::ImageReadAccessor intenAcc(intensityImage, intensityImage->GetSliceData(i, 0, 0));
    distanceArray = (float*)distAcc.GetData();
    amplitudeArray = (float*)amplAcc.GetData();
    intensityArray = (float*)intenAcc.GetData();

    //write (or add) the three slices to the file
    tofNrrdWriter->Add(distanceArray, amplitudeArray, intensityArray);
  }

  tofNrrdWriter->Close(); //close file


  //read in the three images from disc
  mitk::ItkImageFileReader::Pointer fileReader = mitk::ItkImageFileReader::New();
  fileReader->SetFileName(distanceImageFileName);
  fileReader->Update();
  mitk::Image::Pointer distanceImageRead = fileReader->GetOutput();

  fileReader = mitk::ItkImageFileReader::New();
  fileReader->SetFileName(amplitudeImageFileName);
  fileReader->Update();
  mitk::Image::Pointer amplitudeImageRead = fileReader->GetOutput();

  fileReader = mitk::ItkImageFileReader::New();
  fileReader->SetFileName(intensityImageFileName);
  fileReader->Update();
  mitk::Image::Pointer intensityImageRead = fileReader->GetOutput();

  bool readingCorrect = true;
  //  for all frames...
  for(unsigned int j=0; j < numOfFrames; j++)
  {
    //get one slice of each image and compare it
    //original data
    mitk::ImageReadAccessor distAcc(distanceImage, distanceImage->GetSliceData(j, 0, 0));
    mitk::ImageReadAccessor amplAcc(amplitudeImage, amplitudeImage->GetSliceData(j, 0, 0));
    mitk::ImageReadAccessor intenAcc(intensityImage, intensityImage->GetSliceData(j, 0, 0));

    distanceArray = (float*)distAcc.GetData();
    amplitudeArray = (float*)amplAcc.GetData();
    intensityArray = (float*)intenAcc.GetData();

    //data read from disc
    mitk::ImageReadAccessor distReadAcc(distanceImageRead, distanceImageRead->GetSliceData(j, 0, 0));
    mitk::ImageReadAccessor amplReadAcc(amplitudeImageRead, amplitudeImageRead->GetSliceData(j, 0, 0));
    mitk::ImageReadAccessor intenReadAcc(intensityImageRead, intensityImageRead->GetSliceData(j, 0, 0));

    distanceArrayRead = (float*)distReadAcc.GetData();
    amplitudeArrayRead = (float*)amplReadAcc.GetData();
    intensityArrayRead = (float*)intenReadAcc.GetData();

    //for all pixels
    for(unsigned int i=0; i<pixelNumber; i++)
    {
      //compare if input == output
      if(!mitk::Equal(distanceArrayRead[i], distanceArray[i]) ||
         !mitk::Equal(amplitudeArrayRead[i], amplitudeArray[i]) ||
         !mitk::Equal(intensityArrayRead[i], intensityArray[i]))
      {
        readingCorrect = false;
      }
    }
  }

  remove( distanceImageFileName.c_str() );
  remove( amplitudeImageFileName.c_str() );
  remove( intensityImageFileName.c_str() );
  MITK_TEST_CONDITION_REQUIRED(readingCorrect, "Testing if the output values are correct.");

  //delete created image files

  MITK_TEST_END();
}
