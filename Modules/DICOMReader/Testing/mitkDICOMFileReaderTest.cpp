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

#include "mitkDICOMNullFileReader.h"

#include "mitkTestingMacros.h"

class mitkDICOMFileReaderTestClass
{
  public:

static void TestInputFilenames(mitk::DICOMFileReader* reader)
{
  mitk::StringList inputFiles;

  inputFiles.push_back("one");
  inputFiles.push_back("two");
  inputFiles.push_back("three");

  reader->SetInputFiles( inputFiles );

  const mitk::StringList& inputFilesReturned = reader->GetInputFiles();
  MITK_TEST_CONDITION(inputFilesReturned.size() == inputFiles.size(), "Input file list is received")
  MITK_TEST_CONDITION( reader->GetNumberOfOutputs() == 0, "No outputs without analysis")

  // TODO: check strings
}

static void TestOutputsContainInputs(mitk::DICOMNullFileReader* reader)
{
  mitk::StringList inputFiles;

  inputFiles.push_back("one");
  inputFiles.push_back("two");
  inputFiles.push_back("three");

  reader->SetInputFiles( inputFiles );

  reader->AnalyzeInputFiles();

  unsigned int numberOfOutputs = reader->GetNumberOfOutputs();
  MITK_TEST_CONDITION(numberOfOutputs == inputFiles.size(), "DICOMNullFileReader repeats each input as an output")

  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    mitk::DICOMImageBlockDescriptor block = reader->GetOutput(o);

    // TODO: check if all files are in inputs
  }

  // TODO: check that no files are in TWO outputs
  // TODO: check that no files are forgotten

  try
  {
    mitk::DICOMImageBlockDescriptor block = reader->GetOutput( inputFiles.size() );
    MITK_TEST_CONDITION(false, "Invalid indices for GetOutput() should throw exception")
  }
  catch( std::invalid_argument& )
  {
    MITK_TEST_CONDITION(true, "Invalid indices for GetOutput() should throw exception")
  }
}



}; // end test class

int mitkDICOMFileReaderTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkDICOMFileReaderTest");

  mitk::DICOMNullFileReader::Pointer simpleReader = mitk::DICOMNullFileReader::New();
  MITK_TEST_CONDITION_REQUIRED(simpleReader.IsNotNull(), "DICOMNullFileReader can be instantiated.");

  mitkDICOMFileReaderTestClass::TestInputFilenames( simpleReader );

  mitkDICOMFileReaderTestClass::TestOutputsContainInputs( simpleReader );

  MITK_TEST_END();
}
