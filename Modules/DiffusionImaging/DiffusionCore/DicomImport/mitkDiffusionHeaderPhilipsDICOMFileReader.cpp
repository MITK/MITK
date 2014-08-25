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
  //gdcm::Tag philips_gradient_direction( 0x2001, 0x1004 );

  DiffusionImageDICOMHeaderInformation header_info;
  //std::string tagvalue_string;
  //char* pEnd;

  // reveal b-value
  float bvalue = 0;
  if( RevealBinaryTagC( philips_bvalue_tag, gdcmReader.GetFile().GetDataSet(), (char*) &bvalue) )
  {

    header_info.b_value = std::ceil( bvalue );

    if( header_info.b_value == 0)
      header_info.baseline = true;
  }
  else
  {
    MITK_WARN("diffusion.dicomreader.philips") << "No b-value found. Most probably no diffusion-weighted image.";
    return false;
  }

  gdcm::Tag philips_new_bvalue_tag( 0x0018,0x9087 );
  double dbvalue = 0;
  if( RevealBinaryTagC( philips_new_bvalue_tag, gdcmReader.GetFile().GetDataSet(), (char*) &dbvalue) )
  {
    MITK_INFO("philips.dicom.diffusion.bvalue") << dbvalue;
  }

  if( header_info.baseline )
  {
    // no direction in unweighted images
    header_info.g_vector.fill(0);
  }
  else
  {
    MITK_INFO("philips.dicom.diffusion.gradientdir") << "Parsing gradient direction.";

    gdcm::Tag philips_gradient_direction_new( 0x0018, 0x9089 );
    double gr_dir_arr[3] = {1,0,-1};

    if( RevealBinaryTagC( philips_gradient_direction_new, gdcmReader.GetFile().GetDataSet(), (char*) &gr_dir_arr ) )
    {
      MITK_INFO("philips.dicom.diffusion.gradient") << "(" << gr_dir_arr[0] <<"," << gr_dir_arr[1] <<"," <<gr_dir_arr[2] <<")";
    }

    header_info.g_vector.copy_in( &gr_dir_arr[0] );
    if( header_info.g_vector.two_norm() < vnl_math::eps )
    {
      header_info.g_vector.fill(1);
      header_info.isotropic = true;
    }
  }

  this->m_HeaderInformationList.push_back( header_info );

  return true;
}
