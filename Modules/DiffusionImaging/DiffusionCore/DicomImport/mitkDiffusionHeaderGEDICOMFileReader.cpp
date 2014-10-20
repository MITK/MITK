#include "mitkDiffusionHeaderGEDICOMFileReader.h"

mitk::DiffusionHeaderGEDICOMFileReader
::DiffusionHeaderGEDICOMFileReader()
{

}

mitk::DiffusionHeaderGEDICOMFileReader
::~DiffusionHeaderGEDICOMFileReader()
{

}

bool mitk::DiffusionHeaderGEDICOMFileReader
::ReadDiffusionHeader(std::string filename)
{

  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();

  gdcm::Tag ge_bvalue_tag( 0x0043, 0x1039 );
  gdcm::Tag ge_gradient_x( 0x0019, 0x10bb );
  gdcm::Tag ge_gradient_y( 0x0019, 0x10bc );
  gdcm::Tag ge_gradient_z( 0x0019, 0x10bd );

  bool success = true;
  DiffusionImageDICOMHeaderInformation header_info;

  std::string ge_tagvalue_string;
  char* pEnd;

  // start with b-value
  success = RevealBinaryTag( ge_bvalue_tag, gdcmReader.GetFile().GetDataSet(), ge_tagvalue_string );
  // b value stored in the first bytes
  // typical example:  "1000\8\0\0" for bvalue=1000
  //                   "40\8\0\0" for bvalue=40
  // so we need to cut off the last 6 elements
  const char* bval_string = ge_tagvalue_string.substr(0,ge_tagvalue_string.length()-6).c_str();
  header_info.b_value = static_cast<unsigned int>(strtod( bval_string, &pEnd ));

  // now retrieve the gradient direction
  if(success &&
     RevealBinaryTag( ge_gradient_x, gdcmReader.GetFile().GetDataSet(), ge_tagvalue_string ) )
  {
    header_info.g_vector[0] = strtod( ge_tagvalue_string.c_str(), &pEnd );
  }
  else
  {
    success = false;
  }

  if( success &&
      RevealBinaryTag( ge_gradient_y, gdcmReader.GetFile().GetDataSet(), ge_tagvalue_string ) )
  {
    header_info.g_vector[1] = strtod( ge_tagvalue_string.c_str(), &pEnd );
  }
  else
  {
    success = false;
  }

  if( success &&
      RevealBinaryTag( ge_gradient_z, gdcmReader.GetFile().GetDataSet(), ge_tagvalue_string ) )
  {
    header_info.g_vector[2] = strtod( ge_tagvalue_string.c_str(), &pEnd );
  }
  else
  {
    success = false;
  }

  if( success )
  {
    // Fix for (0,0,0) direction in IVIM datasets
    if( header_info.b_value > 0 &&
        header_info.g_vector.two_norm() < vnl_math::eps )
    {
      header_info.g_vector.fill(1);
      header_info.g_vector.normalize();
      header_info.isotropic = true;
    }

    // mark baseline
    if( header_info.b_value == 0 )
      header_info.baseline = true;

    this->m_HeaderInformationList.push_back( header_info );

    header_info.Print();
  }

  return success;



}


