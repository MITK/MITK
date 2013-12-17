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

#include "mitkClassicDICOMSeriesReader.h"

using mitk::DICOMTag;

int main(int argc, char* argv[])
{
  mitk::StringList inputFiles;
  for (int a = 1; a < argc; ++a)
  {
    inputFiles.push_back( std::string(argv[a]) );
  }

  // ----------------- Configure reader -------------------

  // DOES fix tilt when detected
  // DOES group 3D blocks into 3D+t if possible
  mitk::ClassicDICOMSeriesReader::Pointer gdcmReader = mitk::ClassicDICOMSeriesReader::New();

  // ----------------- Load -------------------

  gdcmReader->SetInputFiles( inputFiles );

  MITK_INFO << "Analyzing " << inputFiles.size() << " files ...";
  gdcmReader->AnalyzeInputFiles();
  //gdcmReader->PrintOutputs(std::cout, false);
  MITK_INFO << "Loading " << inputFiles.size() << " files ...";
  gdcmReader->LoadImages();

  unsigned int numberOfOutputs = gdcmReader->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    const mitk::DICOMImageBlockDescriptor block = gdcmReader->GetOutput(o);

    const mitk::DICOMImageFrameList& outputFiles = block.GetImageFrameList();
    mitk::Image::Pointer mitkImage = block.GetMitkImage();

    MITK_INFO << "-------------------------------------------";
    MITK_INFO << "Output " << o << " at " << (void*) mitkImage.GetPointer();
    MITK_INFO << "  Number of files: " << outputFiles.size();
    if (mitkImage.IsNotNull())
    {
      MITK_INFO << "  Dimensions: " << mitkImage->GetDimension(0) << " " << mitkImage->GetDimension(1) << " " << mitkImage->GetDimension(2);
    }
  }
}
