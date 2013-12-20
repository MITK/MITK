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

#include "mitkDICOMFileReaderSelector.h"

using mitk::DICOMTag;

int main(int argc, char* argv[])
{
  mitk::StringList inputFiles;
  for (int a = 1; a < argc; ++a)
  {
    inputFiles.push_back( std::string(argv[a]) );
  }

  // ----------------- Configure reader -------------------
  mitk::DICOMFileReaderSelector::Pointer configSelector = mitk::DICOMFileReaderSelector::New();
  configSelector->LoadBuiltIn3DConfigs(); // a set of compiled in ressources with standard configurations that work well
  configSelector->SetInputFiles( inputFiles );
  mitk::DICOMFileReader::Pointer reader = configSelector->GetFirstReaderWithMinimumNumberOfOutputImages();
  if (reader.IsNull())
  {
    MITK_ERROR << "Could not configure any DICOM reader.. Exiting...";
    return EXIT_FAILURE;
  }

  MITK_INFO << "---- Best reader configuration -----------------";
  MITK_DEBUG << "Found best reader with configuration '" << reader->GetConfigurationLabel() << "'";
  reader->PrintConfiguration( std::cout );
  MITK_INFO << "-------------------------------------------";

  // ----------------- Load -------------------

  reader->SetInputFiles( inputFiles );

  MITK_INFO << "Analyzing " << inputFiles.size() << " files ...";
  reader->AnalyzeInputFiles();
  reader->PrintOutputs(std::cout, false);

  return EXIT_SUCCESS;

  MITK_INFO << "Loading " << inputFiles.size() << " files ...";
  reader->LoadImages();

  unsigned int numberOfOutputs = reader->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    const mitk::DICOMImageBlockDescriptor block = reader->GetOutput(o);

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

  return EXIT_SUCCESS;
}
