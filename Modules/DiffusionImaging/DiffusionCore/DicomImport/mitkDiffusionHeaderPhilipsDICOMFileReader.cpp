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

#include "mitkDiffusionHeaderPhilipsDICOMFileReader.h"

#include <sstream>

mitk::DiffusionHeaderPhilipsDICOMFileReader::DiffusionHeaderPhilipsDICOMFileReader()
{
}

mitk::DiffusionHeaderPhilipsDICOMFileReader::~DiffusionHeaderPhilipsDICOMFileReader()
{

}

bool mitk::DiffusionHeaderPhilipsDICOMFileReader::ReadDiffusionHeader(std::string filename)
{
  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();

  gdcm::Tag philips_bvalue_tag( 0x2001, 0x1003 );
  gdcm::Tag philips_gradient_direction( 0x2001, 0x1004 );

  DiffusionImageDICOMHeaderInformation header_info;
  std::string tagvalue_string;
  //char* pEnd;

  // reveal b-value
  float bvalue = 0;
  if( RevealBinaryTagC( philips_bvalue_tag, gdcmReader.GetFile().GetDataSet(), (char*) &bvalue) )
  {

    header_info.b_value = std::ceil( bvalue );

    if( header_info.b_value == 0)
      header_info.baseline = true;
  }

  // reveal gradient direction
  tagvalue_string.clear();
  if( RevealBinaryTag( philips_gradient_direction, gdcmReader.GetFile().GetDataSet(), tagvalue_string) )
  {
    std::stringstream ssparser( tagvalue_string );
    std::string buffer;

    unsigned int idx=0;
    while( std::getline( ssparser, buffer, '\\' ) )
    {
      std::istringstream substream(buffer);
      substream >> header_info.g_vector[idx++];
    }
  }

  this->m_HeaderInformationList.push_back( header_info );


  return false;
}
