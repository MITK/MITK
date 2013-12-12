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

#include "gdcmScanner.h"
#include "gdcmReader.h"

enum SiemensDiffusionHeaderType {
  SIEMENS_CSA1 = 0,
  SIEMENS_CSA2
};

static SiemensDiffusionHeaderType GetHeaderType( std::string header )
{
  // The CSA2 format begins with the string ‘SV10’, the CSA1 format does not.
  if( header.find("SV10") != std::string::npos )
  {
    return SIEMENS_CSA2;
  }
  else
  {
    return SIEMENS_CSA1;
  }
}

struct Siemens_Header_Format
{
  Siemens_Header_Format( size_t nlen,
                         size_t vm,
                         size_t vr,
                         size_t syngodt,
                         size_t nitems )
    : NameLength( nlen ),
      VM( vm ),
      VR( vr ),
      Syngodt( syngodt ),
      NumItems( nitems ),
      Delimiter( "\0" )
  {}
  size_t NameLength;
  std::string Delimiter;
  size_t VM;
  size_t VR;
  size_t Syngodt;
  size_t NumItems;
};


static std::vector< Siemens_Header_Format > SiemensFormatsCollection;

struct DiffusionImageHeaderInformation
{
  DiffusionImageHeaderInformation()
    : b_value(0)
  {
    g_vector.fill(0);
  }

  void Print()
  {
    MITK_INFO << " DiffusionImageHeaderInformation : \n"
              << "    : b value  : " << b_value << "\n"
              << "    : gradient : " << g_vector << "\n --- \n";
  }

  unsigned int b_value;
  vnl_vector_fixed< double, 3> g_vector;
};

static bool ParseInputString( std::string input, std::vector<double>& values, Siemens_Header_Format format_specs )
{

  int offset = 84;
  int vm = *(input.c_str() + format_specs.NameLength );

  for (int k = 0; k < vm; k++)
  {
    int itemLength = *(input.c_str() + offset + 4);

    int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
    std::string valueString = input.substr( offset+16, itemLength );

    double value = atof( valueString.c_str() );
    values.push_back( value );

    offset += 16+strideSize;
  }

  return true;
}

/**
 * @brief Extract b value from the siemens diffusion tag
 */
static bool ExtractDiffusionTagInformation( std::string tag_value, DiffusionImageHeaderInformation& values)
{
  SiemensDiffusionHeaderType hformat = GetHeaderType( tag_value );
  Siemens_Header_Format specs = SiemensFormatsCollection.at( hformat );

  MITK_INFO << " Header format: " << hformat;
  MITK_INFO << " :: Retrieving b value. ";

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

  // search for GradientDirectionInformation if the bvalue is not null
  if( values.b_value > 0 )
  {
    std::string::size_type tag_position = tag_value.find( "DiffusionGradientDirection", 0 );
    if( tag_position == std::string::npos )
    {
      MITK_INFO << "No gradient direction information. ";
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
    }

  }

  return true;
}

mitk::DiffusionHeaderSiemensDICOMFileReader
::DiffusionHeaderSiemensDICOMFileReader()
{
  Siemens_Header_Format Siemens_CSA1_Format( 64, sizeof(int32_t), 4, sizeof(int32_t), sizeof(int32_t)  );
  Siemens_Header_Format Siemens_CSA2_Format( 64, sizeof(int32_t), 4, sizeof(int32_t), sizeof(int32_t)  );

  SiemensFormatsCollection.push_back( Siemens_CSA1_Format );
  SiemensFormatsCollection.push_back( Siemens_CSA2_Format );
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
  const gdcm::Tag t_sie_diffusion_vec( 0x0029,0x100e );
  const gdcm::Tag t_sie_diffusion2( 0x0029,0x100c );

  std::string siemens_diffusionheader_str;
  if( RevealBinaryTag( t_sie_diffusion, dataset, siemens_diffusionheader_str ) )
  {
    DiffusionImageHeaderInformation values;
    ExtractDiffusionTagInformation( siemens_diffusionheader_str, values );

    values.Print();
  }


}
