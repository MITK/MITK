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
#include "mitkDiffusionHeaderDICOMFileReader.h"

#include "gdcmScanner.h"
#include "gdcmReader.h"

#include <vnl/vnl_math.h>

static bool GetTagFromHierarchy( std::vector< gdcm::Tag > hierarchy, const gdcm::Tag& t_tag, const gdcm::DataSet& dataset, gdcm::DataSet& target)
{
  if( hierarchy.empty() )
    return false;

  const gdcm::DataElement& de = dataset.GetDataElement( hierarchy.at(0) );
  const auto seq = de.GetValueAsSQ();

  if (seq==nullptr)
    return false;

  // last level of hierarchy, retrieve the first apperance
  if( hierarchy.size() == 1 )
  {
    gdcm::Item& item2 = seq->GetItem(1);
    gdcm::DataSet& nestedds2 = item2.GetNestedDataSet();

    const gdcm::DataElement& nde2 = nestedds2.GetDataElement( t_tag );

    if( !nde2.IsEmpty() )
    {
      target = nestedds2;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    if( seq->FindDataElement( hierarchy.at(1) ) )
    {
      for( gdcm::SequenceOfItems::SizeType i=1; i< seq->GetNumberOfItems(); i++ )
      {
        gdcm::Item& item = seq->GetItem(i);
        gdcm::DataSet &nestedds = item.GetNestedDataSet();

        // recursive call
        return GetTagFromHierarchy( std::vector< gdcm::Tag >( hierarchy.begin() + 1, hierarchy.end() ), t_tag, nestedds, target);
      }
    }

    return false;
  }

}

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

  bool retVal = false;

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
    retVal = true;
  }
  else
  {
    const gdcm::Tag t_sie_bvalue( 0x0018, 0x9087);
    const gdcm::Tag t_sie_gradient( 0x0021, 0x1146);

    std::vector< gdcm::Tag > bv_hierarchy;
    bv_hierarchy.push_back( gdcm::Tag(0x5200,0x9230) );
    bv_hierarchy.push_back( gdcm::Tag(0x0018,0x9117) );

    gdcm::DataSet bvalueset;
    GetTagFromHierarchy( bv_hierarchy, t_sie_bvalue, dataset, bvalueset );

    DiffusionImageDICOMHeaderInformation values;
    double dbvalue = 0;
    if( mitk::RevealBinaryTagC( t_sie_bvalue, bvalueset, (char*) &dbvalue) )
    {
      MITK_INFO("siemens.dicom.diffusion.bvalue") << dbvalue;
      values.b_value = std::ceil( dbvalue );

      if( values.b_value == 0)
        values.baseline = true;
    }

    if( !values.baseline )
    {
      std::vector< gdcm::Tag > g_hierarchy;
      g_hierarchy.push_back( gdcm::Tag(0x5200,0x9230) );
      g_hierarchy.push_back( gdcm::Tag(0x0021,0x11fe) );

      GetTagFromHierarchy( g_hierarchy, t_sie_gradient, dataset, bvalueset );

      double gr_dir_arr[3] = {1,0,-1};
      if( mitk::RevealBinaryTagC( t_sie_gradient, bvalueset, (char*) &gr_dir_arr ) )
      {
        MITK_INFO("siemens.dicom.diffusion.gradient") << "(" << gr_dir_arr[0] <<"," << gr_dir_arr[1] <<"," <<gr_dir_arr[2] <<")";
      }

      values.g_vector.copy_in( &gr_dir_arr[0] );
    }

    if( true )
    {
      m_HeaderInformationList.push_back( values );
      retVal = true;
    }
  }

  return retVal;


}
