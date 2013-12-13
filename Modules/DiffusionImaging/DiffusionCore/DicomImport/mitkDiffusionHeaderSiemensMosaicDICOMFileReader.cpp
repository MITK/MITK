#include "mitkDiffusionHeaderSiemensMosaicDICOMFileReader.h"

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::DiffusionHeaderSiemensMosaicDICOMFileReader()
  : DiffusionHeaderSiemensDICOMFileReader()
{

}

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::~DiffusionHeaderSiemensMosaicDICOMFileReader()
{

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
  std::string siemens_diffusionheader_str;
  if( RevealBinaryTag( t_sie_diffusion, dataset, siemens_diffusionheader_str ) )
  {
    mitk::DiffusionImageDICOMHeaderInformation header_values;

    this->ExtractSiemensDiffusionTagInformation( siemens_diffusionheader_str, header_values );

    SiemensDiffusionHeaderType hformat = GetHeaderType( siemens_diffusionheader_str );
    Siemens_Header_Format specs = this->m_SiemensFormatsCollection.at( hformat );


    std::string::size_type tag_position = siemens_diffusionheader_str.find( "NumberOfImagesInMosaic", 0 );
    if( tag_position != std::string::npos )
    {
      std::vector<double> value_array;
      ParseInputString( siemens_diffusionheader_str.substr( tag_position, siemens_diffusionheader_str.size() - tag_position + 1 ),
                        value_array,
                        specs
                        );

      MITK_DEBUG << "Mosaic";
      for( unsigned int i=0; i<value_array.size(); i++)
      {
        MITK_DEBUG << value_array.at(i);
      }
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

    }

    m_HeaderInformationList.push_back( header_values );

  }

}
