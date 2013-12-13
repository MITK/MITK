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

#include "mitkDiffusionDICOMFileReader.h"

#include "mitkDiffusionHeaderDICOMFileReader.h"

#include "mitkDiffusionHeaderSiemensDICOMFileReader.h"
#include "mitkDiffusionHeaderSiemensMosaicDICOMFileReader.h"

mitk::DiffusionDICOMFileReader::DiffusionDICOMFileReader()
{

}

mitk::DiffusionDICOMFileReader::~DiffusionDICOMFileReader()
{

}

bool mitk::DiffusionDICOMFileReader
::LoadImages()
{

}

void mitk::DiffusionDICOMFileReader
::AnalyzeInputFiles()
{
  Superclass::AnalyzeInputFiles();

  // collect output from superclass
  size_t number_of_outputs = this->GetNumberOfOutputs();

  if(number_of_outputs == 0)
  {
    MITK_ERROR << "Failed to parse input, retrieved 0 outputs from SeriesGDCMReader ";
  }

  DICOMImageBlockDescriptor block_0 = this->GetOutput(0);

  MITK_INFO << "Retrieved " << number_of_outputs << "outputs.";

  // collect vendor ID from the first output, first image
  StringList inputFilename;
  DICOMImageFrameInfo::Pointer frame_0 = block_0.GetImageFrameList().at(0);
  inputFilename.push_back( frame_0->Filename );

  gdcm::Scanner gdcmScanner;

  gdcm::Tag t_vendor(0x008, 0x0070);
  gdcm::Tag t_imagetype(0x008, 0x008);

  // add DICOM Tag for vendor
  gdcmScanner.AddTag( t_vendor );
  // add DICOM Tag for image type
  gdcmScanner.AddTag( t_imagetype );
  gdcmScanner.Scan( inputFilename );

  // retrieve both vendor and image type
  std::string vendor = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_vendor );
  std::string image_type = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_imagetype );
  MITK_INFO << "Got vendor: " << vendor << " image type " << image_type;


  mitk::DiffusionHeaderDICOMFileReader::Pointer header_reader;

  // parse vendor tag
  if( vendor.find("SIEMENS") != std::string::npos )
  {
    if( image_type.find("MOSAIC") != std::string::npos )
    {
      header_reader = mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::New();
    }
    else
    {
      header_reader = mitk::DiffusionHeaderSiemensDICOMFileReader::New();
    }

  }
  else if( vendor.find("GE") != std::string::npos )
  {

  }
  else if( vendor.find("PHILIPS") != std::string::npos )
  {

  }
  else
  {
    // unknown vendor
  }

  if( header_reader.IsNull() )
  {
    MITK_ERROR << "No header reader for given vendor. ";
    return;
  }

  bool canread = false;
  for( size_t idx; idx < number_of_outputs; idx++ )
  {
    DICOMImageFrameInfo::Pointer frame = this->GetOutput( idx ).GetImageFrameList().at(0);
    canread = header_reader->ReadDiffusionHeader(frame->Filename);

    MITK_INFO << "Can read ()" << idx << " value: " << canread;

    canread = false;
  }




}


bool mitk::DiffusionDICOMFileReader
::CanHandleFile(const std::string &filename)
{

}

