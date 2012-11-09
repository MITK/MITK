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

#include <mitkToFImageRecorderFilter.h>
#include <mitkImage.h>
#include <mitkImageDataItem.h>
#include "mitkToFNrrdImageWriter.h"
#include "mitkToFImageCsvWriter.h"

// itk includes
#include "itksys/SystemTools.hxx"


mitk::ToFImageRecorderFilter::ToFImageRecorderFilter(): m_RecordingStarted(false), m_ToFImageWriter(0)
{
  m_FileExtension = "";
}

mitk::ToFImageRecorderFilter::~ToFImageRecorderFilter()
{
}

void mitk::ToFImageRecorderFilter::SetFileName(std::string fileName)
{
  std::string name = fileName;
  m_FileExtension = itksys::SystemTools::GetFilenameLastExtension( fileName );
  if(m_FileExtension == ".nrrd")
  {
    m_ToFImageWriter = mitk::ToFNrrdImageWriter::New();
  }
  else if(m_FileExtension == ".csv")
  {
    m_ToFImageWriter = mitk::ToFImageCsvWriter::New();
  }
  else
  {
    throw std::logic_error("The specified file type is not supported, standard file type is .nrrd!");
  }
  int pos = name.find_last_of(".");
  name.insert(pos,"_DistanceImage");
  m_ToFImageWriter->SetDistanceImageFileName(name);
  name = fileName;
  name.insert(pos,"_AmplitudeImage");
  m_ToFImageWriter->SetAmplitudeImageFileName(name);
  name = fileName;
  name.insert(pos,"_IntensityImage");
  m_ToFImageWriter->SetIntensityImageFileName(name);
}

void mitk::ToFImageRecorderFilter::SetImageType(mitk::ToFImageWriter::ToFImageType tofImageType)
{
  m_ToFImageWriter->SetToFImageType(tofImageType);
}

void mitk::ToFImageRecorderFilter::GenerateData()
{
  mitk::Image::Pointer distanceImageInput = this->GetInput(0);
  assert(distanceImageInput);
  mitk::Image::Pointer amplitudeImageInput = this->GetInput(1);
  assert(amplitudeImageInput);
  mitk::Image::Pointer intensityImageInput = this->GetInput(2);
  assert(intensityImageInput);
  // add current data to file stream
  float* distanceFloatData = (float*)distanceImageInput->GetSliceData(0, 0, 0)->GetData();
  float* amplitudeFloatData = (float*)amplitudeImageInput->GetSliceData(0, 0, 0)->GetData();
  float* intensityFloatData = (float*)intensityImageInput->GetSliceData(0, 0, 0)->GetData();
  if (m_RecordingStarted)
  {
    m_ToFImageWriter->Add(distanceFloatData,amplitudeFloatData,intensityFloatData);
  }

  // set outputs to inputs
  this->SetNthOutput(0,distanceImageInput);
  this->SetNthOutput(1,amplitudeImageInput);
  this->SetNthOutput(2,intensityImageInput);
}

void mitk::ToFImageRecorderFilter::StartRecording()
{
  if(m_ToFImageWriter.IsNull())
  {
    throw std::logic_error("ToFImageWriter is unitialized, set filename first!");
    return;
  }
  m_ToFImageWriter->Open();
  m_RecordingStarted = true;
}

void mitk::ToFImageRecorderFilter::StopRecording()
{
  m_ToFImageWriter->Close();
  m_RecordingStarted = false;
}

mitk::ToFImageWriter::Pointer mitk::ToFImageRecorderFilter::GetToFImageWriter()
{
  return m_ToFImageWriter;
}

void mitk::ToFImageRecorderFilter::SetToFImageWriter(mitk::ToFImageWriter::Pointer tofImageWriter)
{
  m_ToFImageWriter = tofImageWriter;
}

void mitk::ToFImageRecorderFilter::SetInput(  mitk::Image* input )
{
  this->SetInput(0,input);
}

void mitk::ToFImageRecorderFilter::SetInput( unsigned int idx,  mitk::Image* input )
{
  if ((input == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
  {
    this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
  }
  else if(idx == 0 || idx == 1 || idx == 2)
  {
    this->ProcessObject::SetNthInput(idx, input);   // Process object is not const-correct so the const_cast is required here
    unsigned int xDim = input->GetDimension(0);
    unsigned int yDim = input->GetDimension(1);
    m_ToFImageWriter->SetToFCaptureWidth(xDim);
    m_ToFImageWriter->SetToFCaptureWidth(yDim);
  }

  this->CreateOutputsForAllInputs();
}

mitk::Image* mitk::ToFImageRecorderFilter::GetInput()
{
  return this->GetInput(0);
}

mitk::Image* mitk::ToFImageRecorderFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFImageRecorderFilter::CreateOutputsForAllInputs()
{
  this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
  for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
    if (this->GetOutput(idx) == NULL)
    {
    DataObjectPointer newOutput = this->MakeOutput(idx);
    this->SetNthOutput(idx, newOutput);
  }
  this->Modified();
}
