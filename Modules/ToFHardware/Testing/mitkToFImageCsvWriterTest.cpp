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
#include <mitkToFImageCsvWriter.h>
#include <mitkImageGenerator.h>
#include <mitkImageSliceSelector.h>
#include "mitkImageReadAccessor.h"

void CloseCsvFile(FILE* outfile)
{
  fclose(outfile);
}

void OpenCsvFile(FILE** outfile, std::string outfileName)
{
  (*outfile) = fopen( outfileName.c_str(), "r" );
  if( !outfile )
  {
    MITK_ERROR << "Error opening outfile: " << outfileName;
    throw std::logic_error("Error opening outfile.");
    return;
  }
}

/**Documentation
 *  test for the class "ToFImageCsvWriter".
 */
int mitkToFImageCsvWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageCsvWriter");
  mitk::ToFImageCsvWriter::Pointer csvWriter = mitk::ToFImageCsvWriter::New();
  MITK_TEST_CONDITION_REQUIRED(csvWriter.GetPointer(), "Testing initialization of test object!");
  MITK_TEST_CONDITION_REQUIRED(csvWriter->GetExtension() == ".csv", "Testing correct initialization of member variable extension!");

  srand(time(0));

  unsigned int dimX = 100 + rand()%100;
  unsigned int dimY = 100 + rand()%100;
  unsigned int pixelNumber = dimX*dimY;
  unsigned int numOfFrames = 1 + rand()%100;

  MITK_INFO<<dimX;
  MITK_INFO<<dimY;
  MITK_INFO<<numOfFrames;

  mitk::Image::Pointer distanceImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames);
  mitk::Image::Pointer amplitudeImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames);
  mitk::Image::Pointer intensityImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, numOfFrames);


  std::string distanceImageFileName("distImg.csv");
  std::string amplitudeImageFileName("amplImg.csv");
  std::string intensityImageFileName("intImg.csv");

  csvWriter->SetDistanceImageFileName(distanceImageFileName);
  csvWriter->SetAmplitudeImageFileName(amplitudeImageFileName);
  csvWriter->SetIntensityImageFileName(intensityImageFileName);
  csvWriter->SetAmplitudeImageSelected(true);
  csvWriter->SetIntensityImageSelected(true);

  csvWriter->SetToFCaptureWidth(dimX);
  csvWriter->SetToFCaptureHeight(dimY);
  csvWriter->SetToFImageType(mitk::ToFImageWriter::ToFImageType3D);

  mitk::ImageSliceSelector::Pointer distanceSelector = mitk::ImageSliceSelector::New();
  mitk::ImageSliceSelector::Pointer amplitudeSelector = mitk::ImageSliceSelector::New();
  mitk::ImageSliceSelector::Pointer intensitySelector = mitk::ImageSliceSelector::New();

  mitk::Image::Pointer tmpDistance;
  mitk::Image::Pointer tmpAmplitude;
  mitk::Image::Pointer tmpIntensity;

  distanceSelector->SetInput(distanceImage);
  amplitudeSelector->SetInput(amplitudeImage);
  intensitySelector->SetInput(intensityImage);

  //buffer
  float* distanceArray;
  float* amplitudeArray;
  float* intensityArray;

  csvWriter->Open(); //open file/stream
  for(unsigned int i = 0; i<numOfFrames; i++)
  { //write values to file/stream
    distanceSelector->SetSliceNr(i);
    distanceSelector->Update();
    mitk::ImageReadAccessor tmpDistAcc(distanceSelector->GetOutput());
    distanceArray = (float*) tmpDistAcc.GetData();

    amplitudeSelector->SetSliceNr(i);
    amplitudeSelector->Update();
    mitk::ImageReadAccessor tmpAmplAcc(amplitudeSelector->GetOutput());
    amplitudeArray = (float*)tmpAmplAcc.GetData();

    intensitySelector->SetSliceNr(i);
    intensitySelector->Update();
    mitk::ImageReadAccessor tmpIntenAcc(intensitySelector->GetOutput());
    intensityArray = (float*)tmpIntenAcc.GetData();

    csvWriter->Add(distanceArray, amplitudeArray, intensityArray);
  }
  csvWriter->Close(); //close file

  FILE* distanceInfile = NULL;
  FILE* amplitudeInfile = NULL;
  FILE* intensityInfile = NULL;

  //open file again
  OpenCsvFile(&(distanceInfile), distanceImageFileName);
  OpenCsvFile(&(amplitudeInfile), amplitudeImageFileName);
  OpenCsvFile(&(intensityInfile), intensityImageFileName);

  float distVal = 0.0, amplVal = 0.0, intenVal = 0.0;
  int dErr = 0, aErr = 0, iErr = 0;
  bool readingCorrect = true;

  //for all frames...
  for(unsigned int j=0; j<numOfFrames; j++)
  {
    distanceSelector->SetSliceNr(j);
    distanceSelector->Update();
    mitk::ImageReadAccessor tmpDistAcc(distanceSelector->GetOutput());
    distanceArray = (float*) tmpDistAcc.GetData();

    amplitudeSelector->SetSliceNr(j);
    amplitudeSelector->Update();
    mitk::ImageReadAccessor tmpAmplAcc(amplitudeSelector->GetOutput());
    amplitudeArray = (float*)tmpAmplAcc.GetData();

    intensitySelector->SetSliceNr(j);
    intensitySelector->Update();
    mitk::ImageReadAccessor tmpIntenAcc(intensitySelector->GetOutput());
    intensityArray = (float*)tmpIntenAcc.GetData();

    //for all pixels
    for(unsigned int i=0; i<pixelNumber; i++)
    {
      if (i==0 && j==0)
      {      //no comma at the beginning of the document
        dErr = fscanf (distanceInfile, "%f", &distVal);
        aErr = fscanf (amplitudeInfile, "%f", &amplVal);
        iErr = fscanf (intensityInfile, "%f", &intenVal);
      }
      else
      {      //comma seperated values now
        dErr = fscanf (distanceInfile, ",%f", &distVal);
        aErr = fscanf (amplitudeInfile, ",%f", &amplVal);
        iErr = fscanf (intensityInfile, ",%f", &intenVal);
      }

      //check if reading error or EOF occurs
      if (dErr==0 || dErr==EOF)
      {
        MITK_TEST_CONDITION_REQUIRED((dErr!=0 && dErr!=EOF), "Testing open and read csv distance file");
      }
      if (aErr==0 || aErr==EOF)
      {
        MITK_TEST_CONDITION_REQUIRED((aErr!=0 && aErr!=EOF), "Testing open and read csv amplitude file");
      }
      if (iErr==0 || iErr==EOF)
      {
        MITK_TEST_CONDITION_REQUIRED((iErr!=0 && iErr!=EOF), "Testing open and read csv intensity file");
      }

      //compare if input == output
      if(!mitk::Equal(distVal,distanceArray[i],0.00001) || !mitk::Equal(amplVal, amplitudeArray[i],0.00001) || !mitk::Equal(intenVal, intensityArray[i],0.00001))
      {
        readingCorrect = false;
      }
    }
  }
  MITK_TEST_CONDITION_REQUIRED(readingCorrect, "Testing if the output values are correct.");

  //check if end of file is reached
  dErr = fscanf (distanceInfile, ",%f", &distVal);
  aErr = fscanf (amplitudeInfile, ",%f", &amplVal);
  iErr = fscanf (intensityInfile, ",%f", &intenVal);
  MITK_TEST_CONDITION_REQUIRED((dErr==EOF), "Testing EOF distance file");
  MITK_TEST_CONDITION_REQUIRED((aErr==EOF), "Testing EOF amplitude file");
  MITK_TEST_CONDITION_REQUIRED((iErr==EOF), "Testing EOF intensity file");

  //close testing files
  CloseCsvFile(distanceInfile);
  CloseCsvFile(amplitudeInfile);
  CloseCsvFile(intensityInfile);

  remove( distanceImageFileName.c_str() );
  remove( amplitudeImageFileName.c_str() );
  remove( intensityImageFileName.c_str() );

  MITK_TEST_END();
}


