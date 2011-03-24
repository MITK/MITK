/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFImageWriter.h>
#include <mitkImageGenerator.h>
#include <mitkImageSliceSelector.h>
#include <mitkPicFileReader.h>
#include <mitkPicFileWriter.h>

/**Documentation
 *  test for the class "ToFImageWriter".
 */
int mitkToFImageWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageWriter");

  //run the test with some unusual parameters
  unsigned int dimX = 255;
  unsigned int dimY = 188;
  unsigned int pixelNumber = dimX*dimY;
  unsigned int numOfFrames = 117; //or numberOfSlices

  //create 3 images filled with random values
  mitk::Image::Pointer distanceImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,0);
  mitk::Image::Pointer amplitudeImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,0);
  mitk::Image::Pointer intensityImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames,0);

  mitk::ToFImageWriter::Pointer tofWriter = mitk::ToFImageWriter::New();

  //file names on the disc
  std::string distanceImageFileName("distImg.pic");
  std::string amplitudeImageFileName("amplImg.pic");
  std::string intensityImageFileName("intImg.pic");

  tofWriter->SetDistanceImageFileName(distanceImageFileName);
  tofWriter->SetAmplitudeImageFileName(amplitudeImageFileName);
  tofWriter->SetIntensityImageFileName(intensityImageFileName);

  MITK_TEST_CONDITION_REQUIRED(distanceImageFileName==tofWriter->GetDistanceImageFileName(), "Testing set/get distance image file name");
  MITK_TEST_CONDITION_REQUIRED(amplitudeImageFileName==tofWriter->GetAmplitudeImageFileName(), "Testing set/get amplitude image file name");
  MITK_TEST_CONDITION_REQUIRED(intensityImageFileName==tofWriter->GetIntensityImageFileName(), "Testing set/get intensity image file name");

  tofWriter->SetCaptureWidth(dimX);
  tofWriter->SetCaptureHeight(dimY);

  MITK_TEST_CONDITION_REQUIRED(dimX==tofWriter->GetCaptureWidth(), "Testing set/get CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(dimY==tofWriter->GetCaptureHeight(), "Testing set/get CaptureHeight");

  tofWriter->SetToFImageType(mitk::ToFImageWriter::ToFImageType2DPlusT);

  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageWriter::ToFImageType2DPlusT==tofWriter->GetToFImageType(), "Testing set/get ToFImageType");

  tofWriter->SetToFImageType(mitk::ToFImageWriter::ToFImageType3D);

  //buffer for each slice
  float* distanceArray;
  float* amplitudeArray;
  float* intensityArray;

  float* distanceArrayRead;
  float* amplitudeArrayRead;
  float* intensityArrayRead;

  tofWriter->Open(); //open file/stream
  //Note: the slices are written out reverse order, because the ToFImageWriter has to write them out immediately.
  //A PicFileWriter would write them out vice versa and the PicFileWriter reads the slices vice versa.
  for(unsigned int i = numOfFrames; i > 0 ; i--)
  { //write values to file/stream
    //The slice index is "i-1", because "for(unsigned int i = numOfFrames-1; i >= 0 ; i--)" does not work for some reason
    distanceArray = (float*)distanceImage->GetSliceData(i-1, 0, 0)->GetData();
    amplitudeArray = (float*)amplitudeImage->GetSliceData(i-1, 0, 0)->GetData();
    intensityArray = (float*)intensityImage->GetSliceData(i-1, 0, 0)->GetData();

    //write (or add) the three slices to the file
    tofWriter->Add(distanceArray, amplitudeArray, intensityArray);
  }
  tofWriter->Close(); //close file

  //read in the three images from disc
  mitk::PicFileReader::Pointer fileReader = mitk::PicFileReader::New();
  fileReader->SetFileName(distanceImageFileName);
  fileReader->Update();
  mitk::Image::Pointer distanceImageRead = fileReader->GetOutput();

  fileReader = mitk::PicFileReader::New();
  fileReader->SetFileName(amplitudeImageFileName);
  fileReader->Update();
  mitk::Image::Pointer amplitudeImageRead = fileReader->GetOutput();

  fileReader = mitk::PicFileReader::New();
  fileReader->SetFileName(intensityImageFileName);
  fileReader->Update();
  mitk::Image::Pointer intensityImageRead = fileReader->GetOutput();

  bool readingCorrect = true;
  //  for all frames...
  for(unsigned int j=0; j<numOfFrames; j++)
  {
    //get one slice of each image and compare it
    //original data
    distanceArray = (float*)distanceImage->GetSliceData(j, 0, 0)->GetData();
    amplitudeArray = (float*)amplitudeImage->GetSliceData(j, 0, 0)->GetData();
    intensityArray = (float*)intensityImage->GetSliceData(j, 0, 0)->GetData();

    //data read from disc
    distanceArrayRead = (float*)distanceImageRead->GetSliceData(j, 0, 0)->GetData();
    amplitudeArrayRead = (float*)amplitudeImageRead->GetSliceData(j, 0, 0)->GetData();
    intensityArrayRead = (float*)intensityImageRead->GetSliceData(j, 0, 0)->GetData();

    //for all pixels
for(unsigned int i=0; i<pixelNumber; i++)
    {
      //compare if input == output
      if(!mitk::Equal(distanceArrayRead[i],distanceArray[i]) ||
         !mitk::Equal(amplitudeArrayRead[i], amplitudeArray[i]) ||
         !mitk::Equal(intensityArrayRead[i], intensityArray[i]))
      {
        readingCorrect = false;
      }
    }
  }
  MITK_TEST_CONDITION_REQUIRED(readingCorrect, "Testing if the output values are correct.");

  //delete created image files
  remove( distanceImageFileName.c_str() );
  remove( amplitudeImageFileName.c_str() );
  remove( intensityImageFileName.c_str() );

  MITK_TEST_END();  
}
