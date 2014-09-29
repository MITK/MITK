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

#include "mitkDICOMReaderConfigurator.h"
#include "mitkDICOMFileReaderSelector.h"

#include "mitkTestingMacros.h"

/**
  \brief Verify serialization ability of DICOMReaderConfigurator.
*/
int mitkDICOMReaderConfiguratorTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkDICOMReaderConfiguratorTest");

  mitk::DICOMFileReaderSelector::Pointer readerSource = mitk::DICOMFileReaderSelector::New();
  readerSource->LoadBuiltIn3DConfigs();
  readerSource->LoadBuiltIn3DnTConfigs();

  mitk::DICOMFileReaderSelector::ReaderList allReaders = readerSource->GetAllConfiguredReaders(); // this already parses XML config for this reader
  for (mitk::DICOMFileReaderSelector::ReaderList::iterator rIter = allReaders.begin();
       rIter != allReaders.end();
       ++rIter)
  {
    mitk::DICOMFileReader::Pointer originalReader = *rIter;

    MITK_TEST_OUTPUT(<< "********************************************************************************");
    MITK_TEST_OUTPUT(<< "Testing reader '" << originalReader->GetConfigurationLabel() << "'" );
    MITK_TEST_OUTPUT(<< "********************************************************************************");
    // ask for an XML serialization of this reader, then compare the XML files.
    mitk::DICOMReaderConfigurator::Pointer serializer = mitk::DICOMReaderConfigurator::New();
    std::string xmlSerialization = serializer->CreateConfigStringFromReader( originalReader.GetPointer() );
    MITK_TEST_CONDITION( !xmlSerialization.empty(), "DICOMReaderConfigurator is able to serialize reader");
    MITK_INFO << "Got serialization:";
    std::cout << xmlSerialization << std::endl;

    mitk::DICOMReaderConfigurator::Pointer creator = mitk::DICOMReaderConfigurator::New();
    try
    {
      mitk::DICOMFileReader::Pointer reconstructedReader = creator->CreateFromUTF8ConfigString(xmlSerialization);
      MITK_TEST_CONDITION( reconstructedReader.IsNotNull(), "DICOMReaderConfigurator is able to create reader from XML");

      MITK_TEST_CONDITION( *reconstructedReader == *originalReader, "Readers are equal before and after serialization" )
    }
    catch(std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Exception from DICOMReaderConfigurator: " << e.what());
    }
  }

  MITK_TEST_END();
}

