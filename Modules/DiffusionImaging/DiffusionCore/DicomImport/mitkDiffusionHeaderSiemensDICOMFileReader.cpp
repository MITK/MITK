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

#include "mitkDiffusionHeaderSiemensDICOMFileReader.h"
#include "mitkDiffusionHeaderSiemensDICOMFileHelper.h"

#include "gdcmScanner.h"
#include "gdcmReader.h"


/**
 * @brief Extract b value from the siemens diffusion tag
 */
bool mitk::DiffusionHeaderSiemensDICOMFileReader
::ExtractSiemensDiffusionTagInformation( std::string tag_value, mitk::DiffusionImageDICOMHeaderInformation& values)
{
  SiemensDiffusionHeaderType hformat = mitk::GetHeaderType( tag_value );
  Siemens_Header_Format specs = this->m_SiemensFormatsCollection.at( hformat );

  MITK_DEBUG << " Header format: " << hformat;
  MITK_DEBUG << " :: Retrieving b value. ";

  std::string::size_type tag_position =
      tag_value.find( "B_value", 0 );

  if( tag_position == std::string::npos )
  {
    MITK_ERROR << "No b value information found. ";
    return false;
  }

  std::string value_string = tag_value.substr( tag_position, tag_value.size() - tag_position + 1 );

  std::vector<double> value_array;
  if( ParseInputString(value_string, value_array, specs) )
  {
    values.b_value = value_array.at(0);
  }
  else
  {
    MITK_INFO("diffusion.dicomreader.siemens") << "No b-value tag found. ";
    return false;
  }

  // search for GradientDirectionInformation if the bvalue is not null
  if( values.b_value > 0 )
  {
    std::string::size_type tag_position = tag_value.find( "DiffusionGradientDirection", 0 );
    // Possibly it is a IVIM dataset, i.e. the gradient direction is not relevant
    // and possibly either not set or set to zero
    if( tag_position == std::string::npos )
    {
      MITK_WARN << "No gradient direction information, but non-zero b-value. Possibly an IVIM dataset. " << "\n"
                << "Setting gradient to (1,1,1).";

      values.isotropic = true;
      values.g_vector.fill(1);
      return false;
    }

    value_array.clear();
    std::string gradient_direction_str = tag_value.substr( tag_position, tag_value.size() - tag_position + 1 );

    if( ParseInputString(gradient_direction_str, value_array, specs) )
    {
      if( value_array.size() != 3 )
      {
        MITK_ERROR << " Retrieved gradient information of length " << value_array.size();
        return false;
      }

      for( unsigned int i=0; i<value_array.size(); i++)
      {
        values.g_vector[i] = value_array.at(i);
      }

      // Test for isotropic data (i.e. IVIM)
      if( values.g_vector.two_norm() < vnl_math::eps )
      {
        values.g_vector.fill(1);
        values.isotropic = true;
      }
    }

  }
  else
  {
    values.baseline = true;
  }

  return true;
}

mitk::DiffusionHeaderSiemensDICOMFileReader
::DiffusionHeaderSiemensDICOMFileReader()
{
  Siemens_Header_Format Siemens_CSA1_Format( 64, sizeof(int32_t), 4, sizeof(int32_t), sizeof(int32_t)  );
  Siemens_Header_Format Siemens_CSA2_Format( 64, sizeof(int32_t), 4, sizeof(int32_t), sizeof(int32_t)  );

  m_SiemensFormatsCollection.push_back( Siemens_CSA1_Format );
  m_SiemensFormatsCollection.push_back( Siemens_CSA2_Format );
}

mitk::DiffusionHeaderSiemensDICOMFileReader
::~DiffusionHeaderSiemensDICOMFileReader()
{

}

bool mitk::DiffusionHeaderSiemensDICOMFileReader
::ReadDiffusionHeader(std::string filename)
{
  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();

  MITK_INFO << " -- Analyzing: " << filename;

  const gdcm::DataSet& dataset = gdcmReader.GetFile().GetDataSet();

  const gdcm::Tag t_sie_diffusion( 0x0029,0x1010 );
  //const gdcm::Tag t_sie_diffusion_vec( 0x0029,0x100e );
  //const gdcm::Tag t_sie_diffusion2( 0x0029,0x100c );

  std::string siemens_diffusionheader_str;
  if( RevealBinaryTag( t_sie_diffusion, dataset, siemens_diffusionheader_str ) )
  {
    DiffusionImageDICOMHeaderInformation values;
    this->ExtractSiemensDiffusionTagInformation( siemens_diffusionheader_str, values );

    m_HeaderInformationList.push_back( values );
  }

  return true;


}
