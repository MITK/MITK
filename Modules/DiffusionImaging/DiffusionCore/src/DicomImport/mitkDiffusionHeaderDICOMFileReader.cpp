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

#include "mitkDiffusionHeaderDICOMFileReader.h"

mitk::DiffusionHeaderDICOMFileReader
::DiffusionHeaderDICOMFileReader()
{

}

mitk::DiffusionHeaderDICOMFileReader
::~DiffusionHeaderDICOMFileReader()
{

}

mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType
mitk::DiffusionHeaderDICOMFileReader
::GetHeaderInformation()
{
  if( m_HeaderInformationList.size() < 1 )
  {
    MITK_WARN << "No information retrieved yet. Call AnalyzeInputFiles first!";
  }

  return m_HeaderInformationList;
}

bool mitk::RevealBinaryTag(const gdcm::Tag tag, const gdcm::DataSet& dataset, std::string& target)
{
  if( dataset.FindDataElement( tag ) )
  {
    MITK_DEBUG << "Found tag " << tag.PrintAsPipeSeparatedString();

    const gdcm::DataElement& de = dataset.GetDataElement( tag );
    target = std::string( de.GetByteValue()->GetPointer(),
                          de.GetByteValue()->GetLength() );
    return true;

  }
  else
  {
    MITK_DEBUG << "Could not find tag " << tag.PrintAsPipeSeparatedString();
    return false;
  }
}

bool mitk::RevealBinaryTagC(const gdcm::Tag tag, const gdcm::DataSet& dataset, char * target_array )
{
  if( dataset.FindDataElement( tag ) )
  {
    MITK_DEBUG << "Found tag " << tag.PrintAsPipeSeparatedString();

    const gdcm::DataElement& de = dataset.GetDataElement( tag );

    size_t bytesize = de.GetValue().GetLength(); //  GetLength();
    //target_array = new char[bytesize];
    memcpy( target_array, de.GetByteValue()->GetPointer(), bytesize);

    return true;

  }
  else
  {
    MITK_DEBUG << "Could not find tag " << tag.PrintAsPipeSeparatedString();
    return false;
  }
}
