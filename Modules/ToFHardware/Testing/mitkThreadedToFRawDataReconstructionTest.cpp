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
#include "mitkThreadedToFRawDataReconstruction.h"
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"

class RawDataDeviceHelperImpl : public mitk::ToFCameraPMDRawDataCamBoardDevice
{
public:
  mitkClassMacro(RawDataDeviceHelperImpl, mitk::ToFCameraPMDRawDataCamBoardDevice);
  itkNewMacro(Self);

  itkSetMacro(CaptureWidth, int);
  itkSetMacro(CaptureHeight, int);

};

/**Documentation
 *  test for the class "mitkThreadedToFRawDataReconstruction".
 */
int mitkThreadedToFRawDataReconstructionTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkThreadedToFRawDataReconstruction");
  mitk::ThreadedToFRawDataReconstruction::Pointer testObject = mitk::ThreadedToFRawDataReconstruction::New();

  MITK_TEST_CONDITION_REQUIRED(testObject.GetPointer(), "Testing initializing of class");
  MITK_TEST_CONDITION_REQUIRED(!testObject->GetInit(), "Testing initial state of GetInit()");
  int width = 176;
  int height = 144;
  int modfreq = 20;
  int sdsize  = width*height*8*sizeof(short);
  testObject->Initialize(width, height, modfreq, sdsize );
  MITK_TEST_CONDITION_REQUIRED(testObject->GetInit(), "Testing state of GetInit() after initialization");

  // generate artificial data
  short* shortSource = new short[sdsize];
  vtkShortArray* channelData = vtkShortArray::New();

  for(long i = 0; i < sdsize; ++i)
  {
    shortSource[i] = i+1;
  }

  // use the helper object to set required variables
  RawDataDeviceHelperImpl::Pointer testHelper = RawDataDeviceHelperImpl::New();
  testHelper->SetCaptureWidth(width);
  testHelper->SetCaptureHeight(height);
  testHelper->GetChannelSourceData(shortSource, channelData);
  testObject->SetChannelData( channelData );
  testObject->Update();

  float* distances = new float[width*height];
  float* amplitudes = new float[width*height];
  float* intensties = new float[width*height];

  // set random value and check if it has changed after filter update
  distances[50] = -111;
  amplitudes[50] = -111;
  intensties[50] = -111;
  float before = distances[50];

  testObject->GetDistances(distances);
  testObject->GetAmplitudes(amplitudes);
  testObject->GetIntensities(intensties);
  MITK_TEST_CONDITION_REQUIRED(before != distances[50], "Testing distance data generation and output");
  MITK_TEST_CONDITION_REQUIRED(before != amplitudes[50], "Testing ampltude data generation and output");
  MITK_TEST_CONDITION_REQUIRED(before != intensties[50], "Testing intensity data generation and output");

  // clean up the mess
  delete[] distances;
  delete[] amplitudes;
  delete[] intensties;
  delete[] shortSource;
  channelData->Delete();
  MITK_TEST_END();
}


