/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMNullFileReader.h"

mitk::DICOMNullFileReader
::DICOMNullFileReader()
:DICOMFileReader()
{
}

mitk::DICOMNullFileReader
::~DICOMNullFileReader()
{
}

mitk::DICOMNullFileReader
::DICOMNullFileReader(const DICOMNullFileReader& other )
:DICOMFileReader(other)
{
}

mitk::DICOMNullFileReader&
mitk::DICOMNullFileReader
::operator=(const DICOMNullFileReader& other)
{
  if (this != &other)
  {
    DICOMFileReader::operator=(other);
  }
  return *this;
}

bool
mitk::DICOMNullFileReader
::operator==(const DICOMFileReader& other) const
{
  return dynamic_cast<const Self*>(&other) != nullptr; // same class, we don't have members
}
void
mitk::DICOMNullFileReader
::InternalPrintConfiguration(std::ostream& os) const
{
  os << "Nothing to configure" << std::endl;
}


void
mitk::DICOMNullFileReader
::AnalyzeInputFiles()
{
  this->ClearOutputs();

  StringList inputFilenames = this->GetInputFiles();
  this->SetNumberOfOutputs( inputFilenames.size() );

  //generates one output for each input
  unsigned int o = 0;
  for (StringList::const_iterator inputIter = inputFilenames.begin();
       inputIter != inputFilenames.end();
       ++o, ++inputIter)
  {
    DICOMImageBlockDescriptor block;
    DICOMImageFrameList outputFrames;
    outputFrames.push_back( DICOMImageFrameInfo::New(*inputIter) );

    block.SetImageFrameList( outputFrames );

    this->SetOutput( o, block );
  }
}

// void AllocateOutputImages();

bool
mitk::DICOMNullFileReader
::LoadImages()
{
  // does nothing
  return true;
}

bool
mitk::DICOMNullFileReader
::CanHandleFile(const std::string& itkNotUsed(filename))
{
  return true; // can handle all
}
