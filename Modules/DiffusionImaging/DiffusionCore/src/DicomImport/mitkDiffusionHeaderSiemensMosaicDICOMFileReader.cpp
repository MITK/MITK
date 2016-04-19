#include "mitkDiffusionHeaderSiemensDICOMFileHelper.h"
#include "mitkDiffusionHeaderSiemensMosaicDICOMFileReader.h"

#include <sstream>

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::DiffusionHeaderSiemensMosaicDICOMFileReader()
  : DiffusionHeaderSiemensDICOMFileReader()
{

}

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::~DiffusionHeaderSiemensMosaicDICOMFileReader()
{

}

void mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::RetrieveMosaicInformation(std::string filename)
{
  // retrieve also mosaic information
  if( !this->ReadDiffusionHeader( filename ) )
  {
    MITK_ERROR << "Using MOSAIC Reader for non-mosaic files ";
  }

  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();
  const gdcm::DataSet& dataset = gdcmReader.GetFile().GetDataSet();

  // (0018,0088) DS [2.5]                                    #   4, 1 SpacingBetweenSlices
  // important for mosaic data
  std::string spacing_between_slices;
  if( RevealBinaryTag( gdcm::Tag(0x0018, 0x0088 ), dataset, spacing_between_slices  ) )
  {
    std::istringstream ifs_spacing(spacing_between_slices );
    ifs_spacing >> this->m_MosaicDescriptor.spacing[2];
  }

  //(0028,0030) DS [2.5\2.5]                                #   8, 2 PixelSpacing
  std::string inplace_spacing;
  if( RevealBinaryTag( gdcm::Tag(0x0028, 0x0030 ), dataset, inplace_spacing  ) )
  {
      std::stringstream iplss( inplace_spacing );
      std::string buffer;
      unsigned int idx=0;
      while( std::getline( iplss, buffer, '\\' ) )
      {
        std::istringstream substream(buffer);
        substream >> this->m_MosaicDescriptor.spacing[idx++];
      }
   }

  // (0020,0032) DS [-802.51815986633\-801.18644070625\-30.680992126465] #  50, 3 ImagePositionPatient
  std::string position_patient;
  if( RevealBinaryTag( gdcm::Tag( 0x0028, 0x0032), dataset, position_patient) )
  {
    std::stringstream ppass( inplace_spacing );
    std::string buffer;
    unsigned int idx=0;
    while( std::getline( ppass, buffer, '\\' ) )
    {
      std::istringstream substream(buffer);
      substream >> this->m_MosaicDescriptor.origin[idx++];
    }
  }

}

bool mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::ReadDiffusionHeader(std::string filename)
{
  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();

  MITK_INFO << " -- Analyzing: " << filename;

  const gdcm::DataSet& dataset = gdcmReader.GetFile().GetDataSet();

  const gdcm::Tag t_sie_diffusion( 0x0029,0x1010 );
  const gdcm::Tag t_sie_diffusion_alt( 0x0029,0x1110 );
  std::string siemens_diffusionheader_str;

  if( RevealBinaryTag( t_sie_diffusion, dataset, siemens_diffusionheader_str )
      || RevealBinaryTag( t_sie_diffusion_alt, dataset, siemens_diffusionheader_str) )
  {
    mitk::DiffusionImageMosaicDICOMHeaderInformation header_values;
    // wait for success
    if( !this->ExtractSiemensDiffusionTagInformation( siemens_diffusionheader_str, header_values ))
      return false;

    mitk::SiemensDiffusionHeaderType hformat = GetHeaderType( siemens_diffusionheader_str );
    Siemens_Header_Format specs = this->m_SiemensFormatsCollection.at( hformat );

    std::string::size_type tag_position = siemens_diffusionheader_str.find( "NumberOfImagesInMosaic", 0 );
    if( tag_position != std::string::npos )
    {
      std::vector<double> value_array;
      ParseInputString( siemens_diffusionheader_str.substr( tag_position, siemens_diffusionheader_str.size() - tag_position + 1 ),
                        value_array,
                        specs
                        );

      header_values.n_images = value_array[0];
      this->m_MosaicDescriptor.nimages = value_array[0];
    }

    tag_position = siemens_diffusionheader_str.find("SliceNormalVector", 0);
    if( tag_position != std::string::npos )
    {
      std::vector<double> value_array;
      ParseInputString( siemens_diffusionheader_str.substr( tag_position, siemens_diffusionheader_str.size() - tag_position + 1 ),
                        value_array,
                        specs
                        );

      MITK_DEBUG << "SliceNormal";
      for( unsigned int i=0; i<value_array.size(); i++)
      {
        MITK_DEBUG << value_array.at(i);
      }

      if( value_array.size() > 2 )
      {
        header_values.slicenormalup = (value_array[2] > 0);
        this->m_MosaicDescriptor.slicenormalup = ( value_array[2] > 0);
      }

    }



    m_HeaderInformationList.push_back( header_values );

  }

  return (m_HeaderInformationList.size() > 0);

}
